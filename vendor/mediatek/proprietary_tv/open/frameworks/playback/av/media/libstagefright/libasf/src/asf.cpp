/*  libasf - An Advanced Systems Format media file parser
*  Copyright (C) 2006-2010 Juho Vähä-Herttua
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*   *  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA   */

#define LOG_NDEBUG 0
#include <stdlib.h>
#include <stdio.h>
#include <cutils/properties.h>
#include "asfparser.h"

#undef LOG_TAG
#define LOG_TAG "ASF ASFParser"

static bool bVerboseLog = false;

#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGV(...) ALOGV_IF(bVerboseLog, __VA_ARGS__)

/******************************************************************************
**
**      ASF Parser APIs
**
******************************************************************************/
ASFParser::ASFParser(void* source, asf_io_read_func_ptr read, asf_io_write_func_ptr write, asf_io_seek_func_ptr seek)
{
    //initialize all member variables
    file = NULL;
    iError = ASF_SUCCESS;

    // initialize asf stream
    asf_iostream_t stream;
    stream.read = read;
    stream.write = write;
    stream.seek = seek;
    stream.source = source;

    // add for DLNA
    mHasParsedIndexTab = false;
    //Initialize asf parser library
    file = asf_open_file(&stream);
    if (!file)
    {
        ALOGE("Error failed to Initialize ASF parser");
        iError = ASF_INSUFFICIENT_DATA; //check for current error
    }

    bVerboseLog = (bool)property_get_int32("vendor.asfparser.verbose.log", 0);
}

ASFParser::~ASFParser()
{
    //release all the memory
    asf_close();
}

ASFErrorType ASFParser::IsAsfFile()
{
    int status;
    status = asf_init();
    if (status < 0)
            return ASF_FILE_HDR_READ_ERR;

    return ASF_SUCCESS;
}


asf_file_t *
ASFParser::asf_open_file(asf_iostream_t *pStream)
{
    asf_file_t *asffile = NULL;

    asffile = asf_open_cb(pStream);
    if (!asffile)
            return NULL;

    asffile->filename = NULL;//filename;

    return asffile;
}

asf_file_t *
ASFParser::asf_open_cb(asf_iostream_t *iostream)
{
    asf_file_t *asffile= NULL;
    int i;

    if (!iostream)
            return NULL;

    asffile = (asf_file_t *)calloc(1, sizeof(asf_file_t));
    if (!asffile)
            return NULL;

    asffile->index_parsed=false;

    asffile->filename = NULL;
    asffile->iostream.read = iostream->read;
    asffile->iostream.write = iostream->write;
    asffile->iostream.seek = iostream->seek;
    asffile->iostream.source = iostream->source;

    asffile->header = NULL;
    asffile->data = NULL;
    asffile->simple_index = NULL;
    asffile->index = NULL;
    //asffile->index = NULL;


    for (i=0; i < ASF_MAX_STREAMS; i++) {
            asffile->streams[i].type = ASF_STREAM_TYPE_NONE;
            asffile->streams[i].flags = ASF_STREAM_FLAG_NONE;
            asffile->streams[i].properties = NULL;
            asffile->streams[i].extended = NULL;
            asffile->streams[i].current_packet = 0;
    }
    asffile->hasDRMObj=false;


    return asffile;
}

int
ASFParser::asf_init()
{
    int tmp;

    if (!file)
    {
        ALOGD("return ASF_ERROR_INTERNAL.L:%d", __LINE__);
        return ASF_ERROR_INTERNAL;
    }

    tmp = asf_parse_header();
    if (tmp < 0) {
        ALOGV("error parsing header: %d", tmp);
        return tmp;
    }
    file->position += tmp;
    file->data_position = file->position;

    tmp = asf_parse_data();
    if (tmp < 0) {
        ALOGE("error parsing data object: %d", tmp);
        return tmp;
    }
    file->position += tmp;

    for (tmp = 0; tmp < ASF_MAX_STREAMS; tmp++)
    {
        if (file->streams[tmp].type != ASF_STREAM_TYPE_NONE)
        {
           ALOGD("stream %d of type %d found!\n", tmp, file->streams[tmp].type);
        }
    }

    return 0;
}

void
ASFParser::asf_close()
{
    if (file)
    {
        int i;

        asf_free_header(file->header);
        if (file->data) free(file->data);
        if (file->simple_index)
        {
            if (file->simple_index->entries) free(file->simple_index->entries);
            free(file->simple_index);
        }
        if(file->index)
        {
            if(file->index->specifiers_entry) free(file->index->specifiers_entry);
            if(file->index->index_block->index_entry) free(file->index->index_block->index_entry);
            if(file->index->index_block) free(file->index->index_block);
            free(file->index);
        }

        if (file->iostream.source)
        {
            //fclose (file->iostream.pStreamFp);
            file->iostream.source = NULL;
        }

        for (i=0; i < ASF_MAX_STREAMS; i++)
        {
            if (file->streams[i].properties) free(file->streams[i].properties);
            if (file->streams[i].extended) free(file->streams[i].extended);
        }
        free(file);
    }
}

asf_packet_t *
ASFParser::asf_packet_create()
{
    asf_packet_t *ret = NULL;

    ret = (asf_packet_t *)calloc(1, sizeof(asf_packet_t));
    if (!ret)
    return NULL;

    asf_data_init_packet(ret);

    return ret;
}

int
ASFParser::asf_get_stream_packet(asf_packet_t *packet, uint32_t aStreamID, bool isForward)
{
    uint64_t new_position;
    int64_t seek_position;
    int tmp;

    if (!file ||
        file->streams[aStreamID].type == ASF_STREAM_TYPE_NONE ||
        file->streams[aStreamID].current_packet >= file->data_packets_count)
    {
        ALOGE("asf_get_stream_packet: ASF_ERROR_INTERNAL file=%p track=%d type=%d current_packet=%llu\n",
            file, aStreamID,
            file ? file->streams[aStreamID].type : 0,
            file ? file->streams[aStreamID].current_packet : 0);
        return ASF_ERROR_INTERNAL;
    }

    do {
        /* calculate stream specific packet position */
        new_position = file->data->packets_position + file->streams[aStreamID].current_packet * file->packet_size;
        ALOGV("asf_get_stream_packet: aStreamID=%d, packets_position=%llu, current_packet =%llu, packet_size=%u\n",
            aStreamID, file->data->packets_position, file->streams[aStreamID].current_packet, file->packet_size);

        //seek to stream specific packets position
        if(file->position != new_position)
        {
            seek_position = file->iostream.seek(file->iostream.source, new_position);
            if (seek_position < 0 || seek_position != (int64_t)new_position) {
                ALOGE("asf_get_stream_packet:error 2,seek_position=%lld,new_position=%lld\n",seek_position,new_position);
                return ASF_ERROR_SEEK;
            }
            file->position = new_position;
            file->packet = file->streams[aStreamID].current_packet;
        }

        ALOGV("asf_get_stream_packet: aStreamID=%d,file->position=%lld,file->packet =%lld\n",aStreamID,file->position,file->packet);

        tmp = asf_get_packet(packet);

        ALOGV("asf_get_stream_packet: stream %d current packet index is %lld\n",aStreamID, file->streams[aStreamID].current_packet);

        if (isForward) {
            file->streams[aStreamID].current_packet++;
        }
        else {
            file->streams[aStreamID].current_packet--;
        }
    }while (tmp == ASF_ERROR_INVALID_LENGTH);

    return tmp;
}


int
ASFParser::asf_get_packet(asf_packet_t *packet)
{
    int tmp;
    //ALOGE("asf_get_packet file->packet=%lld",file->packet);

    if (!file || !packet)
    return ASF_ERROR_INTERNAL;

    if (!(file->flags&ASF_FLAG_BROADCAST) && (file->packet >= file->data_packets_count))
    {
        ALOGE("asf_get_packet:EOS \n");
        //ALOGE("asf_get_packet:error 1,file->packet=%lld,file->data_packets_count=%lld\n",file->packet,file->data_packets_count);
        return 0;
    }

    tmp = asf_data_get_packet(packet);
    if (tmp < 0)
    {
        ALOGE("asf_get_packet:error 2,tmp=%d\n",tmp);
        return tmp;
    }

    file->position += tmp;
    file->packet++;

    return tmp;
}

void
ASFParser::asf_packet_destroy(asf_packet_t *packet)
{
    if (!packet) return;
    asf_data_free_packet(packet);
    free(packet);
}

int
ASFParser::asf_get_track_pts(uint32_t track, int64_t* pi8_pts, bool isForward)
{
    int ret = 0;
    uint32_t payload_number;

    if (!file){
        ALOGE("asf_get_track_pts: invalid file\n");
        return ASF_ERROR_INTERNAL;
    }

    ALOGV("asf_get_track_pts[%u]: before isForward=%d current_packet=%llu current_payload=%u\n",
        track, isForward, file->streams[track].current_packet, payload_number);

    asf_packet_t* packet = asf_packet_create();
    asf_payload_t* payload = NULL;

    do {
        if ((ret = asf_get_stream_packet(packet, track, isForward)) <= 0) {
            ALOGE("asf_get_track_pts: get packet fail %d\n", ret);
            break;
        }

        for (payload_number = 0; payload_number < packet->payload_count; payload = NULL, payload_number++) {
            payload = &packet->payloads[payload_number];
            if (track == payload->stream_number) {
                *pi8_pts = payload->pts;
                ret = ASF_PARSE_SUCCESS;
                ALOGV("asf_get_track_pts[%u]: pts=%lld current_packet=%llu current_payload=%u\n",
                    track, *pi8_pts, file->streams[track].current_packet, payload_number);
                break;
            }
        }
    } while (!payload);

    if (packet) {
        asf_packet_destroy(packet);
        packet = NULL;
    }

    return ret;
}

uint64_t
ASFParser::asf_seek_file(int64_t msec)
{
    uint64_t packet_number = 0;
    bool seek_done = false;

    if (!file){
        ALOGE("asf_seek_file: invalid file\n");
        return packet_number;
    }

    /* For some special files, seek to 0s by simple_index table is not accurate
     * we would prefer to use time+packet size calc method instead of simple_index table
     */
    if (msec && file->simple_index)
    {
        uint32_t simple_index_entry;

        /* Fetch current packet from index entry structure
         * entry_time_interval between each index entry in 100-nanosecond units
         * index_entry is second unit
         */
        simple_index_entry = msec * 10000 / file->simple_index->entry_time_interval;

        if (simple_index_entry >= file->simple_index->entry_count)
        {
            ALOGE("asf_seek_file: out of simple index\n");
            seek_done=false;
        }
        else
        {
            seek_done=true;
            packet_number = file->simple_index->entries[simple_index_entry].packet_index;
            ALOGI("asf_seek_file: simple_index_entry=%d\n", simple_index_entry);
        }

    }

    if((!seek_done) && file->index)
    {
        uint32_t  index_entry = msec / file->index->index_entry_time_interval;    //ms unit
        if (index_entry >= file->index->index_block->index_entry_count)
        {
            ALOGE("asf_seek_file: out of index\n");
            seek_done=false;
        }
        else
        {
            seek_done=true;
            packet_number = (file->index->index_block->index_entry[index_entry].offset + file->index->index_block->block_positions)/file->packet_size;
            ALOGI("asf_seek_file: index_entry=%d\n", index_entry);
        }
    }

    if(!seek_done && (file->real_duration > 0) && (file->packet_size > 0) && (file->data->size > 0))
    {
        /* convert msec into bytes per second and divide with packet_size */
        packet_number = (msec * file->data->size /(file->real_duration))/file->packet_size;

        if(packet_number > file->data_packets_count)
        {
            ALOGE("asf_seek_file: can not find the right packet=%lld, data_packets_count=%lld\n", packet_number, file->data_packets_count);
        }

        seek_done =true;
        ALOGI("asf_seek_file: seek done by time and packet size");
    }

    return packet_number;
}

int
ASFParser::asf_seek_video_stream(uint32_t track, int64_t* pts)
{
    uint32_t payload_number;

    if (!file){
        ALOGE("asf_seek_video_stream: invalid file\n");
        return ASF_ERROR_INTERNAL;
    }

    if (file->streams[track].type != ASF_STREAM_TYPE_VIDEO){
        ALOGE("asf_seek_video_stream: not video track\n");
        return ASF_ERROR_INTERNAL;
    }

    ALOGI("asf_seek_video_stream[%u]: before seek current_packet=%llu current_payload=%u\n",
        track, file->streams[track].current_packet, file->streams[track].current_payload);

    int ret;
    asf_packet_t* packet = asf_packet_create();
    asf_payload_t* payload = NULL;

    /* For each video stream in an ASF file, there should be on instance of the Simple Index Object.
     * The order of the Simple Index Objects should be identical to the order of the video streams
     * based on their stream numbers
     * Now ONLY the bit-streams with ONE or NONE video stream are supported well
     */
    do {
        if ((ret = asf_get_stream_packet(packet, track, true)) <= 0) {
            ALOGE("asf_seek_video_stream: get packet fail %d\n", ret);
            break;
        }

        for (payload_number = 0; payload_number < packet->payload_count; payload = NULL, payload_number++) {
            payload = &packet->payloads[payload_number];
            if (track == payload->stream_number && payload->key_frame) {
                *pts = payload->pts;
                file->streams[track].current_packet -= 1;
                file->streams[track].current_payload = payload_number;
                ALOGI("asf_seek_video_stream[%u]: pts=%lld current_packet=%llu current_payload=%u\n",
                        track, *pts, file->streams[track].current_packet, file->streams[track].current_payload);
                ret = ASF_PARSE_SUCCESS;
                break;
            }
            else {
                ret = ASF_ERROR_INTERNAL;
            }
        }
    } while (!payload);

    if (packet) {
        asf_packet_destroy(packet);
        packet = NULL;
    }

    return ret;
}

int
ASFParser::asf_seek_audio_stream(int64_t msec, uint32_t track, int64_t* pi8_pts)
{
    int ret                    = 0;
    int64_t pts                = 0;
    int64_t low_pts            = -1;
    int64_t high_pts           = -1;
    uint64_t low_packet        = 0;
    uint64_t high_packet       = 0;
    uint64_t delta_packet      = 0;
    uint32_t payload_number    = 0;
    bool isForward             = true;

    if (!file){
        ALOGE("asf_seek_audio_stream: invalid file\n");
        return ASF_ERROR_INTERNAL;
    }

    if (file->streams[track].type != ASF_STREAM_TYPE_AUDIO){
        ALOGE("asf_seek_audio_stream: not video track\n");
        return ASF_ERROR_INTERNAL;
    }

    ALOGI("asf_seek_audio_stream[%u]: before seek current_packet=%llu current_payload=%u\n",
        track, file->streams[track].current_packet, file->streams[track].current_payload);

    /* search the target packet */
    do {
        if (msec <= 0) {
            ALOGI("asf_seek_audio_stream[%u]: seek msec=%u skip searching\n", track, msec);
            break;
        }

        ret = asf_get_track_pts(track, &pts, isForward);
        if (ret != ASF_PARSE_SUCCESS) {
            ALOGE("asf_seek_audio_stream: get track[%d] pts fail ret %d\n", track, ret);
            return ret;
        }

        if (isForward) {
            file->streams[track].current_packet -= 1;
        }
        else {
            file->streams[track].current_packet += 1;
        }

        delta_packet = MAX(DELTA_PACKET(file, pts - msec), 1);

        ALOGV("asf_seek_audio_stream: track[%d] isForward=%d pts=%lld current_packet=%llu delta_packet=%llu\n",
            track, isForward, pts, file->streams[track].current_packet, delta_packet);

        if ((low_pts > -1 && high_pts > -1) &&
            (low_packet == file->streams[track].current_packet ||
             high_packet == file->streams[track].current_packet)) {
            file->streams[track].current_packet = low_packet;
            ALOGI("asf_seek_audio_stream[%u]: hit! low_pts=%lld high_pts=%lld current_packet=%llu\n",
                track, low_pts, high_pts, file->streams[track].current_packet);
            break;
        }
        else if (pts >= msec && pts <= msec + 100) {
            ALOGI("asf_seek_audio_stream[%u]: hit! pts=%lld current_packet=%llu\n",
                track, pts, file->streams[track].current_packet);
            break;
        }
        else if (pts < msec) {
            low_pts = pts;
            low_packet = file->streams[track].current_packet;
            file->streams[track].current_packet = low_packet + delta_packet;
            isForward = true;
        }
        else {
            high_pts = pts;
            high_packet = file->streams[track].current_packet;
            file->streams[track].current_packet = high_packet - delta_packet;
            isForward = false;
        }

        ALOGV("asf_seek_audio_stream: track[%d] isForward=%d low_packet=%llu high_packet=%llu low_pts=%lld high_pts=%lld\n",
            track, isForward, low_packet, high_packet, low_pts, high_pts);
    } while (true);

    asf_packet_t* packet = asf_packet_create();
    asf_payload_t* payload = NULL;

    /* search the target payload */
    do {
        if ((ret = asf_get_stream_packet(packet, track, true)) <= 0) {
            ALOGE("asf_seek_audio_stream: get packet fail %d\n", ret);
            break;
        }

        for (payload_number = 0; payload_number < packet->payload_count; payload = NULL, payload_number++) {
            payload = &packet->payloads[payload_number];
            if (track == payload->stream_number && payload->pts >= msec) {
                *pi8_pts = payload->pts;
                file->streams[track].current_packet -= 1;
                file->streams[track].current_payload = payload_number;
                ret = ASF_PARSE_SUCCESS;
                ALOGI("asf_seek_audio_stream[%u]: pts=%lld current_packet=%llu current_payload=%u\n",
                    track, *pi8_pts, file->streams[track].current_packet, file->streams[track].current_payload);
                break;
            }
        }
    } while (!payload);

    if (packet) {
        asf_packet_destroy(packet);
        packet = NULL;
    }

    return ret;
}

int
ASFParser::asf_seek_to_msec(int64_t msec, uint32_t track, int64_t* pts)
{
    int ret = 0;
    uint64_t packet_number=0;

    if (!file){
        ALOGE("asf_seek_to_msec: invalid file\n");
        return ASF_ERROR_INTERNAL;
    }

    if (msec > (int64_t)(file->real_duration)) {
        ALOGE("asf_seek_to_msec: out of duration=%lld\n", file->real_duration );
        return ASF_ERROR_SEEK;
    }

    if (track < ASF_MAX_STREAMS) {
        ALOGI("asf_seek_to_msec[%u]: seek msec=%u current_packet=%llu current_payload=%u\n",
            track, msec, file->streams[track].current_packet, file->streams[track].current_payload);
    }

    if (file->simple_index == NULL && file->index == NULL && msec != 0) {
        if (asf_parse_index() != ASF_PARSE_SUCCESS) {
            ALOGE("asf_seek_to_msec: parse index fail!!!\n");
            return ASF_ERROR_SEEK;
        }
    }

    packet_number = asf_seek_file(msec);

    if (track < ASF_MAX_STREAMS) {
        file->streams[track].current_packet = packet_number;

        if (file->streams[track].type == ASF_STREAM_TYPE_VIDEO) {
            ret = asf_seek_video_stream(track, pts);
        }
        else if (file->streams[track].type == ASF_STREAM_TYPE_AUDIO) {
            ret = asf_seek_audio_stream(msec, track, pts);
        }

        ALOGI("asf_seek_to_msec[%u]: pts=%lld current_packet=%llu current_payload=%u\n",
            track, *pts, file->streams[track].current_packet, file->streams[track].current_payload);
    }

    return ret;
}

asf_metadata_t *
ASFParser::asf_header_get_metadata()
{
    if (!file || !file->header)
    return NULL;

    return asf_header_metadata(file->header);
}


asf_metadata_entry_t*
ASFParser::asf_findMetaValueByKey(asf_metadata_t*  meta, char* key,int  in_len)
{
    asf_metadata_t* ret=    meta;
    if(ret==NULL)
    {
        ALOGE("[ASF_ERROR]: no meta!");
        return NULL;
    }
    ALOGI("content_count=%d, extended_count=%d, metadata_count=%d,ret->metadatalib_count-=%d",
                ret->content_count,ret->extended_count,ret->metadata_count,ret->metadatalib_count);

    if(ret->content_count>0)
    {
        for(int i=0;i<ret->content_count;i++)
        {
            if(ret->content[i].key &&  !strncmp(key,ret->content[i].key, in_len))
            {
                return  &(ret->content[i]);
            }
        }
    }

    if(ret->extended_count>0)
    {
        for(int i=0;i<ret->extended_count;i++)
        {
             if(ret->extended[i].key && !strncmp(key,ret->extended[i].key, in_len))
             {
                return  &(ret->extended[i]) ;
             }
         }
    }

    if(ret->metadata_count>0)
    {
        for(int i=0;i<ret->metadata_count;i++)
        {
            if(ret->metadata[i].key && !strncmp(key,ret->metadata[i].key, in_len))
            {
                return  &(ret->metadata[i])  ;
            }
         }
    }

    if(ret->metadatalib_count>0)
    {
        for(int i=0;i<ret->metadatalib_count;i++)
        {
            if(ret->metadatalib[i].key &&  !strncmp(key,ret->metadatalib[i].key, in_len))
            {
                return  &(ret->metadatalib[i]);
            }
        }
    }

    return NULL;
}


void
ASFParser::asf_parse_WMPicture(uint8_t* WMPicture,uint32_t size, uint32_t* dataoff)
{

    uint8_t picType=0;
    uint32_t picDataLen=0;
    uint32_t cur_des_data_len=0;
    uint8_t* cur_pos = WMPicture;
    *dataoff = 0;

    picType = *(cur_pos);
    cur_pos = cur_pos+1;

    picDataLen = ASFByteIO::asf_byteio_getDWLE(cur_pos);
    if(picDataLen<size)
    {
        *dataoff = size - picDataLen;
        ALOGI("picType=%d,picDataLen=%d,off=%d\n",picType,picDataLen,*dataoff);
    }
    else
    {
         *dataoff = 0;
         ALOGE("Error: asf_parse_WMPicture:picDataLen=%d",picDataLen);
    }
}

void
ASFParser::asf_header_destroy()
{
    if (!file)
    return;

    asf_free_header(file->header);
    file->header = NULL;
}

void
ASFParser::asf_metadata_destroy(asf_metadata_t *metadata)
{
    if (metadata != NULL)
    {
        asf_header_free_metadata(metadata);
    }
}

uint8_t
ASFParser::asf_get_stream_count()
{
    uint8_t ret = 0;
    uint8_t i;

    if (!file)
    return 0;

    for (i = 0; i < ASF_MAX_STREAMS; i++)
    {
        if (file->streams[i].type != ASF_STREAM_TYPE_NONE)
        ret = i;//???why not ret++???
    }

    return ret;
}

int
ASFParser::asf_is_broadcast()
{
    if (!file)
    return 0;

    return (file->flags & ASF_FLAG_BROADCAST);
}

int
ASFParser::asf_is_seekable()
{
    if (!file)
        return 0;

    if(!(file->flags & ASF_FLAG_SEEKABLE))
    {
        ALOGE("asf_is_seekable:error 1:!(file->flags & ASF_FLAG_SEEKABLE)\n");
        if(file->flags & ASF_FLAG_BROADCAST) {
            ALOGE("asf_is_seekable:file->flags & ASF_FLAG_BROADCAST");
            return ASF_FLAG_BROADCAST;
        }
        return 0;
    }
    else
    {
        if(file->simple_index == NULL)
        {
            int i, audiocount;

            audiocount = 0;
            for(i=0; i<ASF_MAX_STREAMS; i++)
            {
                if (file->streams[i].type == ASF_STREAM_TYPE_NONE)
                        continue;

                // Non-audio files are not seekable without index
                if (file->streams[i].type == ASF_STREAM_TYPE_AUDIO){
                        audiocount++;
                }
            }
        }
    }
    return ASF_FLAG_SEEKABLE;
}

asf_stream_t *
ASFParser::asf_get_stream(uint8_t track)
{
    if (!file || track >= ASF_MAX_STREAMS)
    return NULL;

    return &file->streams[track];
}

uint64_t
ASFParser::asf_get_file_size()
{
    if (!file)
    return 0;

    return file->file_size;
}

uint64_t
ASFParser::asf_get_creation_date()
{
    if (!file)
    return 0;

    return file->creation_date;
}

uint64_t
ASFParser::asf_get_data_packets()
{
    if (!file)
    return 0;

    return file->data_packets_count;
}

uint64_t
ASFParser::asf_get_duration()
{
    if (!file)
    return 0;

    return (file->real_duration);// ms unit
}


uint32_t
ASFParser::asf_get_max_bitrate()
{
    if (!file)
    return 0;

    return file->max_bitrate;
}

uint32_t
ASFParser::asf_get_packet_size()
{
    if (!file)
    return 0;

    return file->packet_size;
}

int
ASFParser::asf_get_track_num(asf_stream_type_t type)
{
    int ret = 0;
    int i;

    if (!file)
    return 0;

    for (i = 0; i < ASF_MAX_STREAMS; i++)
    {
        if (file->streams[i].type == type)
        {
            ret = i;
            break;
        }
    }

    return ret;
}
//add by qian

uint64_t
ASFParser::asf_get_preroll_ms()
{
    if (!file)
    return 0;

    return file->preroll; //ms
}


uint8_t
ASFParser::asf_check_simple_index_obj()
{
    if (!file)
    {
        return 0;
    }

    if(file->simple_index) //ms
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t
ASFParser::asf_parse_check_hasDRM()
{
    if (file->hasDRMObj)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

guid_type_t ASFParser::asf_get_sub_format(uint8_t *data, int size)
{
    asf_guid_t guid;
    guid_type_t ret;

    if (data == NULL)
    {
        ALOGE("asf_get_sub_format: data null\n");
        return GUID_UNKNOWN;
    }

    if (size < WAVE_FORMAT_EXTENSIBLE_SIZE)
    {
        ALOGE("asf_get_sub_format: invalid cbSize=%d\n", size);
        return GUID_UNKNOWN;
    }

    ASFByteIO::asf_byteio_getGUID(&guid, data + 6 /* skip WORD Samples & DWORD dwChannelMask */);

    ALOGI("asf_get_sub_format: guid=%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X\n",
        guid.v1, guid.v2, guid.v3, guid.v4[0], guid.v4[1], guid.v4[2],
        guid.v4[3], guid.v4[4], guid.v4[5], guid.v4[6], guid.v4[7]);

    ret = asf_guid_get_sub_format_type(&guid);

    ALOGI("asf_get_sub_format: guid_type_t=%d\n", ret);

    return ret;
}
