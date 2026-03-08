/*
 * Copyright (c) 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef PRESENCE_DETECTION_H
#define PRESENCE_DETECTION_H

#define INTERVAL_LOW 100u
#define INTERVAL_HIGH 1000u
#define FRAME_RATE_LOW 1u
#define FRAME_RATE_HIGH 10u

//Radar config params - DO NOT CHANGE
#define SC1233A_RADAR_INTER_1FPS	(1000u)
#define SC1233A_RADAR_INTER_10FPS	(100u)
#define SC1233A_RADAR_INTER_BETA_1FPS	(52u)
#define SC1233A_RADAR_INTER_BETA_10FPS	(205u)
#define SC1233A_RADAR_FRAME_RATE_1FPS	(1u)
#define SC1233A_RADAR_FRAME_RATE_10FPS	(10u)


#define DISTANCE_THRESHOLD (40u)
#define INGRESS_SCORE_THRESHOLD (8u)
#define INGRESS_POWER_THRESHOLD_MULTIPLIER (4u)

#define ZERO_OFFSET (14u)
#define BIN_RESOLUTION (225u) 

// Interval in seconds.
#define HISTOGRAM_PERIOD (120u)
#define FRAME_RATE_HIGH_COUNT_PERIOD_SAMPLES (HISTOGRAM_PERIOD*FRAME_RATE_HIGH)
#define FRAME_RATE_LOW_CALL_FAN_DETECTION_COUNT (480u) // Call fan detection every 8 minutes.

#define DIS_DIM (27u)  // Distance_bins dimension

#define MAX_NUMBER_OF_TARGETS (5u)

/* Parameters for empty room with fan detection */
#define NON_EMPTY_BIN_THRESHOLD (60u)
#define SUM_BIN_FAN_THRESHOLD (200u)
#define TRIVIAL_PRESENCE_THRESHOLD (5u)
#define MINIMUM_FAN_DISTANCE_BIN (3u)

#define LOW_POWER_THRESHOLD (16u)
#define LOW_POWER_SUM_THRESHOLD (100u)
#define LOW_POWER_BIN_THRESHOLD (3u)

/* Debug logs? */

enum sc1233a_log_lv {
	SC1233A_LOG_ERR,
	SC1233A_LOG_INFO,
	SC1233A_LOG_DEBUG,
	SC1233A_LOG_MAX
};

static enum sc1233a_log_lv sc1233a_log_level = SC1233A_LOG_ERR;

const u16 range_bin_to_dist[DIS_DIM] = {14, 37, 59, 81, 104, 126, 149, 171, 193, 216, 238, 260, 283, 305, 327, 350, 372, 395, 417, 439, 462, 484, 506, 529, 551, 573, 596};

/*radar parameter structure: this includes the radar parameters that can be updated from the SNI API*/
struct parameter_radar_dynamic
{
	u16	interval;
	u8	beta;
};

/*application structure*/
struct presence_handle
{
	u8	status;			// presence detection result
	u8	frame_counter;		// frame_counter of ingress validation window
	u16	no_presence_counter;	// frame_counter of egress validation window
	u8	counter_score;		// score to validate the ingress
	u8      power_score;           // Accumulate power of peaks marked as ingress candidates
	u8	rate_change_flag;	// flag to control rate change
	u8	initialized;		// flag to check if presence_handle has been initialized.
	/* data structures for fan detection */
	u16 rx1_distance_histogram_10fps[DIS_DIM];
	u16 rx1_distance_histogram_lowpower_10fps[DIS_DIM];
	/* for logging, keep a backup */
	u16 rx1_distance_histogram_10fps_previous[DIS_DIM];
	u16 rx1_distance_histogram_lowpower_10fps_previous[DIS_DIM];
	u8 fan_bin_idx[DIS_DIM];
	u8 fan_bins[DIS_DIM];
	u8 number_of_fan_bins;
	u16 frame_index; // This is a counter to keep track of 2 min windows over which histograms are counted.
	u8 ten_fps_for_fan_detection_only;
	u8 empty_room_fan_detected_with_low_power; // Set to 1 if empty room with fan detected using below threshold 10fps statistics
};


/*algorithm parameter structure*/
struct parameter_app
{
	u8 presence_threshold;		// peak threshold
	u8 turnOn_delay;		// timewindow (# of frames) to justify ingress
	u8 trigger_outdate_counter;	// timer (# of frames) when an old ingress trigger expires
	u8 turnOff_delay;		// timewindow (# of frames) to justify egress
};

/* Initialize fan bins */
void init_fan_bins(struct presence_handle *handle)
{
	int i;
	for(i=0;i<DIS_DIM;i++)
	{
	    handle->fan_bins[i] = DIS_DIM;
	    handle->fan_bin_idx[i] = 0;
	}
	handle->number_of_fan_bins = 0;
}

/* Appends a guard bin on either side of detected fan bins. For example: If candidate fan bins is [17,18], this returns [16,17,18,19]*/
void append_fan_bins(struct presence_handle *handle, int start_idx, int end_idx)
{
	int start_dim, end_dim,i;
	end_dim = (end_idx+1==DIS_DIM)?end_idx:end_idx+1;
	start_dim = start_idx - 1;
	for(i=start_dim;i<=end_dim;i++)
	{
	    if (handle->number_of_fan_bins > 0)
	    {
	        if (i <= handle->fan_bins[handle->number_of_fan_bins-1])
	            continue;
	    }
	    
	    handle->fan_bins[handle->number_of_fan_bins] = i;
	    handle->number_of_fan_bins += 1;
	}
}

/* Considers fan bin candidates and saves valid fan bins after appending guard bins. */
int return_fan_bins(struct presence_handle *handle, u8 candidates_length)
{
	int i=0;
	int count = 0;
	int start_idx, end_idx, block;
	if (candidates_length == 0)
	{
	// Nothing to do.
	    return 1;
	}
	start_idx = 0;
	end_idx = 0;
	block = 0;
	while(i<DIS_DIM)
	{
	    if (handle->fan_bin_idx[i] == 1 && i < MINIMUM_FAN_DISTANCE_BIN)
	    {
	        i += 1;
	        count += 1;
	        if (count == candidates_length)
	            return 1;
	        continue;
	    }   
	    if (handle->fan_bin_idx[i] == 1)
	    {
	        count += 1;       
	        if (block == 0)
	        {
	            start_idx = i;
	            end_idx = i;
	            block = 1;
	            if (count == candidates_length)
	            {
	                append_fan_bins(handle, start_idx, end_idx);
	                break;
	            }
	            i += 1;
	            continue;
	        }
	        if (block == 1)
	        {
	            end_idx = i;
	            if (count == candidates_length)
	            {
	                append_fan_bins(handle, start_idx, end_idx);
	                break;
	            }
	            i += 1;
	            continue;
	        }
	    }
	    else{
	        if (block == 1)
	        {
	            append_fan_bins(handle, start_idx, end_idx);
	            block = 0;
	            i += 1;
	            continue;
	        }
	        i += 1;
	    } 
	}
	return 0;
}

/* Copies PCAM and LP-PCAM histogram to preserve previous values for logging*/
void backup_histograms(struct presence_handle *handle)
{
    int j;
    for(j=0;j<DIS_DIM;j++)
    {
        handle->rx1_distance_histogram_10fps_previous[j] = handle->rx1_distance_histogram_10fps[j];
        handle->rx1_distance_histogram_lowpower_10fps_previous[j] = handle->rx1_distance_histogram_lowpower_10fps[j];
    }
}

/* Detects whether a fan is running in an empty room */
/* Returns 0 if empty_room_fan_detection is true, 2 if presence due to motion, 1 otherwise */
/* Clears presence status if empty_room_fan_detection is true */
int return_empty_room_fan_detection(struct presence_handle *handle)
{
    
    int i, ret;
    int non_empty_bin_count = 0;
    int fan_bin_count = 0;
    backup_histograms(handle);
    for(i=0;i<DIS_DIM;i++)
    {
        non_empty_bin_count = (handle->rx1_distance_histogram_10fps[i]>NON_EMPTY_BIN_THRESHOLD)?non_empty_bin_count+1:non_empty_bin_count;
        if (handle->rx1_distance_histogram_10fps[i]>SUM_BIN_FAN_THRESHOLD)
        {
            handle->fan_bin_idx[i] = 1;
            fan_bin_count += 1;
        }
    }
    ret = return_fan_bins(handle, fan_bin_count);
    if (sc1233a_log_level == SC1233A_LOG_INFO)
    {
	    printk("sc1233:PCAM:Fan_Bins, Fan_bin_count, ret_value:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],"
	    "[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],%d,%d",
		handle->rx1_distance_histogram_10fps[0],
		handle->rx1_distance_histogram_10fps[1],
		handle->rx1_distance_histogram_10fps[2],
		handle->rx1_distance_histogram_10fps[3],
		handle->rx1_distance_histogram_10fps[4],
		handle->rx1_distance_histogram_10fps[5],
		handle->rx1_distance_histogram_10fps[6],
		handle->rx1_distance_histogram_10fps[7],
		handle->rx1_distance_histogram_10fps[8],
		handle->rx1_distance_histogram_10fps[9],
		handle->rx1_distance_histogram_10fps[10],
		handle->rx1_distance_histogram_10fps[11],
		handle->rx1_distance_histogram_10fps[12],
		handle->rx1_distance_histogram_10fps[13],
		handle->rx1_distance_histogram_10fps[14],
		handle->rx1_distance_histogram_10fps[15],
		handle->rx1_distance_histogram_10fps[16],
		handle->rx1_distance_histogram_10fps[17],
		handle->rx1_distance_histogram_10fps[18],
		handle->rx1_distance_histogram_10fps[19],
		handle->rx1_distance_histogram_10fps[20],
		handle->rx1_distance_histogram_10fps[21],
		handle->rx1_distance_histogram_10fps[22],
		handle->rx1_distance_histogram_10fps[23],
		handle->rx1_distance_histogram_10fps[24],
		handle->rx1_distance_histogram_10fps[25],
		handle->rx1_distance_histogram_10fps[26],
		handle->fan_bins[0],
		handle->fan_bins[1],
		handle->fan_bins[2],
		handle->fan_bins[3],
		handle->fan_bins[4],
		handle->fan_bins[5],
		handle->fan_bins[6],
		handle->fan_bins[7],
		handle->fan_bins[8],
		handle->fan_bins[9],
		handle->fan_bins[10],
		handle->fan_bins[11],
		handle->fan_bins[12],
		handle->fan_bins[13],
		handle->fan_bins[14],
		handle->fan_bins[15],
		handle->fan_bins[16],
		handle->fan_bins[17],
		handle->fan_bins[18],
		handle->fan_bins[19],
		handle->fan_bins[20],
		handle->fan_bins[21],
		handle->fan_bins[22],
		handle->fan_bins[23],
		handle->fan_bins[24],
		handle->fan_bins[25],
		handle->fan_bins[26],handle->number_of_fan_bins, ret);
	}
    if (non_empty_bin_count - fan_bin_count > TRIVIAL_PRESENCE_THRESHOLD)
    {
        handle->empty_room_fan_detected_with_low_power = 0;
        return 2;
    }
    if (ret == 0)
    {
        handle->status = 0;
        handle->empty_room_fan_detected_with_low_power = 0;
        if (handle->ten_fps_for_fan_detection_only == 1)
        {
            init_fan_bins(handle);
        }
    }
    return ret;
}

/*Detects whether a fan is running in an empty room using low power statistics at 10 fps*/
/* Returns 0 if empty_room_fan_detection is true, 1 otherwise*/
/* If empty_room_fan_detection is true, presence is cleared */
int return_empty_room_fan_detection_low_power(struct presence_handle *handle)
{
	int j, ret;
	int count = 0;
	for(j=0;j<DIS_DIM;j++)
	{
	    count = (handle->rx1_distance_histogram_lowpower_10fps[j]>LOW_POWER_SUM_THRESHOLD)?count+1:count;
	}
	ret = (count>=LOW_POWER_BIN_THRESHOLD)?0:1;
	if (ret == 0)
	{
	    handle->status = 0;
	    handle->empty_room_fan_detected_with_low_power = 1;
	}
	if (sc1233a_log_level == SC1233A_LOG_INFO)
	{
		printk("sc1233:LP_PCAM:Count:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],%d",
		handle->rx1_distance_histogram_lowpower_10fps[0],
		handle->rx1_distance_histogram_lowpower_10fps[1],
		handle->rx1_distance_histogram_lowpower_10fps[2],
		handle->rx1_distance_histogram_lowpower_10fps[3],
		handle->rx1_distance_histogram_lowpower_10fps[4],
		handle->rx1_distance_histogram_lowpower_10fps[5],
		handle->rx1_distance_histogram_lowpower_10fps[6],
		handle->rx1_distance_histogram_lowpower_10fps[7],
		handle->rx1_distance_histogram_lowpower_10fps[8],
		handle->rx1_distance_histogram_lowpower_10fps[9],
		handle->rx1_distance_histogram_lowpower_10fps[10],
		handle->rx1_distance_histogram_lowpower_10fps[11],
		handle->rx1_distance_histogram_lowpower_10fps[12],
		handle->rx1_distance_histogram_lowpower_10fps[13],
		handle->rx1_distance_histogram_lowpower_10fps[14],
		handle->rx1_distance_histogram_lowpower_10fps[15],
		handle->rx1_distance_histogram_lowpower_10fps[16],
		handle->rx1_distance_histogram_lowpower_10fps[17],
		handle->rx1_distance_histogram_lowpower_10fps[18],
		handle->rx1_distance_histogram_lowpower_10fps[19],
		handle->rx1_distance_histogram_lowpower_10fps[20],
		handle->rx1_distance_histogram_lowpower_10fps[21],
		handle->rx1_distance_histogram_lowpower_10fps[22],
		handle->rx1_distance_histogram_lowpower_10fps[23],
		handle->rx1_distance_histogram_lowpower_10fps[24],
		handle->rx1_distance_histogram_lowpower_10fps[25],
		handle->rx1_distance_histogram_lowpower_10fps[26],count);
	}
	return ret;
}

/*change the radar parameters*/
/*intialize algorithm parameters*/
void parameter_radar_dynamic_update(struct parameter_radar_dynamic *para, u16 interval)
{
	/*change the parameters of radar configurations by the specified frame interval. Two radar parameter sets are given for two frame intervals, i.e., 100 [ms] and 1000 [ms]*/
	/*
	Input:
	para: pointer of struct parameter_radar_dynamic, which includes the tunable radar parameters
	interval: uint16_t type, the interval [ms]*/
	para->interval = interval;
	para->beta = (interval == INTERVAL_HIGH) ? 52 : 205;
}

/*intialize algorithm parameters*/
int parameter_app_update(struct parameter_app *para, u8 frame_rate)
{
	/*change the parameter set of application algorithm by the specified frame rate*/
	/*
	Input:
	para: pointer of struct parameter_app , which includes the frame rate-specified algorithm parameters
	interval: uint16_t type, the interval [ms]

	Return:
	1 or 0: 1 indicates that the radar parameter set for the specified interval is valid. 0 indicates that the radar parameter set for the specified interval is not defined.
	para will be updated by the parameter set specified by frame rate
	*/
	switch (frame_rate)
	{
	case FRAME_RATE_LOW:
		para->presence_threshold = 19;
		para->turnOn_delay = (u8)(1 * frame_rate); // This is a don't care.
		para->trigger_outdate_counter = (u8)(2 * frame_rate);
		para->turnOff_delay = (u8)(10 * frame_rate);
		break;
	case FRAME_RATE_HIGH:
		para->presence_threshold = 23;
		//para->turnOn_delay = (u8)(1 * frame_rate);
		para->turnOn_delay = INGRESS_SCORE_THRESHOLD;
		para->trigger_outdate_counter = (u8)(15 * frame_rate/10);
		para->turnOff_delay = (u8)(5 * frame_rate);
		break;
	default:
		return -1;
	}
	return 1;
}

/* Initialize pcam histograms */
void init_histogram_counts(struct presence_handle *handle)
{
	int i;
	for(i=0;i<DIS_DIM;i++)
	{
	    handle->rx1_distance_histogram_10fps[i] = 0;
	    handle->rx1_distance_histogram_lowpower_10fps[i] = 0;
	}
}

/* Initialize pcam previous histograms */
void init_previous_histogram_counts(struct presence_handle *handle)
{
	int i;
	for(i=0;i<DIS_DIM;i++)
	{
	    handle->rx1_distance_histogram_10fps_previous[i] = 0;
	    handle->rx1_distance_histogram_lowpower_10fps_previous[i] = 0;
	}
}

/*function to initialize the presence_handle*/
void presence_init(struct presence_handle *handle, u8 fan_bins[], u8 number_of_fan_bins, u8 empty_room_fan_detected)
{
	/*initialize the presence_handle with all zero values for attribute*/
	/*
	Input:
	handle: pointer of struct presence_handle

	Return:
	None
	*/
	int j;
	handle->status = 0;
	handle->frame_counter = 0;
	handle->no_presence_counter = 0;
	handle->counter_score = 0;
	handle->power_score = 0;
	handle->rate_change_flag = 0;
	handle->initialized = 1;
	// Initialize fan detection data structures.
	init_histogram_counts(handle);
	init_previous_histogram_counts(handle);
	for(j=0;j<DIS_DIM;j++)
	{
	    handle->fan_bins[j] = fan_bins[j];
	    handle->fan_bin_idx[j] = 0;  // We don't have to worry about the state of this array as the only purpose of this is to help compute fan bins.
	}
	handle->number_of_fan_bins = number_of_fan_bins;
	handle->frame_index = 0;
	handle->ten_fps_for_fan_detection_only = 0;
	handle->empty_room_fan_detected_with_low_power = empty_room_fan_detected;
}

/* function to fulfill the presence detection*/
void presence_detection(struct presence_handle *handle_2D, struct parameter_app *para, struct parameter_radar_dynamic *para_radar, u8 max_peak_level, u32 distance, u32 rx1_distances[], u8 rx1_peaks[], u8 number_of_valid_peaks)
{
	/* Input:
	   handle_2D: pointer of presence_handle
	   para: pointer of parameter_app
	   data: pointer of data_frame
	   Return:
	   None
	*/
	int i,j,tmp,ret;
	int contains_non_fan_distance=0;
	int contains_non_spurious_peaks=0;
	int dist_idx; // Histogram indices.	
	/* Keep track of 2 minutes after which decide if we need to run inanimate mobile object detection pipeline.*/
	if (para_radar->interval == INTERVAL_LOW)
	{
		if (handle_2D->frame_index >= FRAME_RATE_HIGH_COUNT_PERIOD_SAMPLES)
		{
			{
				/* Re-initialize fan distances */
				for(j=0;j<DIS_DIM;j++)
				{
				    handle_2D->fan_bins[j] = DIS_DIM;
				    handle_2D->fan_bin_idx[j] = 0;
				}
				handle_2D->number_of_fan_bins = 0;
				/** If empty room fan running and fan detection called regular 10 fps mode
				    reset presence
				    re-initialize histograms.*/
				ret = return_empty_room_fan_detection(handle_2D);
				if (ret == 1)
				{
				    ret = return_empty_room_fan_detection_low_power(handle_2D);
				}
			}
			if (ret > 0 && handle_2D->ten_fps_for_fan_detection_only == 1)
			{
				// Empty room fan detected -- Could be empty or static. Either case, presence is marked 0 and we don't go back to 1 fps.
				// No empty room fan detected -- Go back to 1 fps, if we got here from 1 fps.
				handle_2D->ten_fps_for_fan_detection_only = 0;
				parameter_radar_dynamic_update(para_radar, INTERVAL_HIGH);
				parameter_app_update(para, FRAME_RATE_LOW);
				handle_2D->frame_index = 0;
				init_histogram_counts(handle_2D);
				init_fan_bins(handle_2D);
				handle_2D->rate_change_flag = 1;
				return;
			}
			handle_2D->ten_fps_for_fan_detection_only = 0;
			handle_2D->frame_index = 0;
			init_histogram_counts(handle_2D);
		}
	}
	else{
		if (handle_2D->frame_index >= FRAME_RATE_LOW_CALL_FAN_DETECTION_COUNT)
			{
				if(handle_2D->no_presence_counter == para->turnOff_delay - 1)
				{
				    if (max_peak_level < para->presence_threshold)
				    {
				        handle_2D->status = 0;
					parameter_radar_dynamic_update(para_radar, INTERVAL_LOW);
					parameter_app_update(para, FRAME_RATE_HIGH);
					handle_2D->rate_change_flag = 1;
					handle_2D->no_presence_counter = 0;
					/* Rate change -- reset the fan detection datastructures*/
					handle_2D->frame_index = 0;
					init_histogram_counts(handle_2D);
					init_fan_bins(handle_2D);
					return;
				    }
				}
				parameter_radar_dynamic_update(para_radar, INTERVAL_LOW);
				parameter_app_update(para, FRAME_RATE_HIGH);
				handle_2D->no_presence_counter = 0;
				handle_2D->frame_index = 0;
				init_histogram_counts(handle_2D);
				init_fan_bins(handle_2D);
				handle_2D->ten_fps_for_fan_detection_only = 1;
				handle_2D->rate_change_flag = 1;
				return;
			}
	}
	handle_2D->frame_index += 1;
	/*check whether there exists a detection trigger to be validated*/
	if (handle_2D->frame_counter > 0)
	{
		handle_2D->frame_counter += 1;
	}
	/*no valid measurements*/
	if ((max_peak_level < para->presence_threshold) && (handle_2D->ten_fps_for_fan_detection_only==0))
	{
		/*check whether the candidate detection trigger has been outdated or not -- to filter out spurious ingress peaks*/
		if ((handle_2D->status == 0) && (handle_2D->frame_counter > (para->trigger_outdate_counter) + 1))
		{
			handle_2D->counter_score = 0;
			handle_2D->power_score = 0;
			handle_2D->frame_counter = 0;
		}
		/*increment the non-presence counter when TV is on*/
		if (handle_2D->status == 1)
		{
			handle_2D->no_presence_counter += 1;
			if (handle_2D->no_presence_counter >= para->turnOff_delay)
			{
				if (para_radar->interval == INTERVAL_HIGH) {
					handle_2D->status = 0;
					parameter_radar_dynamic_update(para_radar, INTERVAL_LOW);
					parameter_app_update(para, FRAME_RATE_HIGH);
				}
				else {
					parameter_radar_dynamic_update(para_radar, INTERVAL_HIGH);
					parameter_app_update(para, FRAME_RATE_LOW);
				}
				handle_2D->rate_change_flag = 1;
				handle_2D->no_presence_counter = 0;
				/* Rate change -- reset the fan detection datastructures*/
				handle_2D->frame_index = 0;
				init_histogram_counts(handle_2D);
				init_fan_bins(handle_2D);
				return;
			}
		}
		return;
	}
	/* Accumulate low power bins.*/
	if ((max_peak_level < para->presence_threshold) && (para_radar->interval == INTERVAL_LOW))
	{
	    for (j=0;j<MAX_NUMBER_OF_TARGETS;j++)
	    {
	        if (rx1_peaks[j] <= LOW_POWER_THRESHOLD)
	            break;
	        if (rx1_peaks[j] >= para->presence_threshold)
	            continue;
	        dist_idx = ((rx1_distances[j] - ZERO_OFFSET)*10)/BIN_RESOLUTION;
		tmp = ((rx1_distances[j] - ZERO_OFFSET)*10)%BIN_RESOLUTION;
		dist_idx = (tmp < BIN_RESOLUTION/2) ? dist_idx : dist_idx + 1;
		handle_2D->rx1_distance_histogram_lowpower_10fps[dist_idx] += 1;		     
	    }
	    return;
	}
	/* We have come here -- which implies max_peak >= threshold. Here let's just stick to rx1. Can be possible rx1 max peak is lower than threshold*/
	for (j=0;j<number_of_valid_peaks;j++)
	{
		/* Don't consider spurious peaks from corrupting true ingress events */
		if ((handle_2D->status == 0) && (rx1_distances[j] < DISTANCE_THRESHOLD))
			continue;
		contains_non_spurious_peaks = 1;
		/* Let's populate the histograms and check for fan bins if we are at 10 fps */
		if (para_radar->interval == INTERVAL_LOW)
		{
			dist_idx = ((rx1_distances[j] - ZERO_OFFSET)*10)/BIN_RESOLUTION;
			tmp = ((rx1_distances[j] - ZERO_OFFSET)*10)%BIN_RESOLUTION;
			dist_idx = (tmp < BIN_RESOLUTION/2) ? dist_idx : dist_idx + 1;
			handle_2D->rx1_distance_histogram_10fps[dist_idx] += 1;
			/* Check if peak is not from existing detected fan bins*/
			if ((handle_2D->number_of_fan_bins > 0 || handle_2D->empty_room_fan_detected_with_low_power == 1) && (handle_2D->ten_fps_for_fan_detection_only == 0) && (contains_non_fan_distance == 0))
			{
				for(i=0;i<handle_2D->number_of_fan_bins;i++)
				{
					if (dist_idx == handle_2D->fan_bins[i])
					{
						break;
					}
				}
				if (i == handle_2D->number_of_fan_bins)
				{
				    contains_non_fan_distance = 1;
				    if (handle_2D->status == 0)
				    {
				        handle_2D->power_score += rx1_peaks[j] - para->presence_threshold;
				    }
				}
			}
		}
	}	
	if(handle_2D->ten_fps_for_fan_detection_only == 0)
	{
		/* If no non-fan related peaks in the frame, then skip it*/
		if ((handle_2D->number_of_fan_bins > 0 || handle_2D->empty_room_fan_detected_with_low_power == 1) && (contains_non_fan_distance == 0) && para_radar->interval == INTERVAL_LOW)
		{
			return;
		}
		
		/* If only spurious peaks, then skip frame*/
		if (contains_non_spurious_peaks == 0)
		{
			return;
		}
		/*reset the none presence counter when there is valid non-fan, non-spurious measurement*/
		handle_2D->no_presence_counter = 0;
		/*when state is on, continue with on */
		if (handle_2D->status == 1)
		{
			return;
		}
		if (handle_2D->frame_counter == 0)
		{
			handle_2D->frame_counter = 1;
		}
		
		/*increment the score to confirm the ingress*/
		handle_2D->counter_score += 1;
		/*validate the ingress event*/
		if (handle_2D->frame_counter > (para->turnOn_delay))    // Wait at least INGRESS_SCORE_THRESHOLD+1 frames.
		{
			// Decide within 16 frames.
			if ((handle_2D->frame_counter <= (para->trigger_outdate_counter) + 1) && (handle_2D->counter_score >= INGRESS_SCORE_THRESHOLD))
			{
				if (handle_2D->number_of_fan_bins == 0 && handle_2D->empty_room_fan_detected_with_low_power == 0)
				{
				    handle_2D->status = 1;
				} else {
				    if (handle_2D->power_score < (handle_2D->counter_score*INGRESS_POWER_THRESHOLD_MULTIPLIER) + 1)
				        return;
				    handle_2D->status = 1;
				}	
			}
			handle_2D->counter_score = 0;
			handle_2D->power_score = 0;
			handle_2D->frame_counter = 0;
		}
	}
return;
}
#endif

