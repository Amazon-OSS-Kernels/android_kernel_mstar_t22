#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <MsRawIO.h>
#include <android_image.h>
#include <MsSystem.h>
#include <MsMmc.h>
#include <mmc.h>

#if(ENABLE_MMC==1)
#define FLASH_RAW_DATA_TARGET E_RAW_DATA_IN_MMC
#elif (ENABLE_NAND_FLASH==1)
#define FLASH_RAW_DATA_TARGET E_RAW_DATA_IN_NAND
#else
#error
#endif

#define WITH_PADDING_SIZE(size, page)    (((size)+(page)-1) & (~((page)-1)))

#ifndef MAX_IMAGE_SIZE
#define MAX_IMAGE_SIZE (25*1024*1024)
#endif

uint32_t get_timestamp(void)
{
    return MsSystemGetBootTime();
}

static int get_size_of_partition(const char* partition, uint32_t* out_size_in_bytes) {
    struct mtd_device *dev;
    struct part_info *partInfo;
    uint32_t* partSize = 0;
    uint32_t* partOffset = 0;
    uint8_t partNum;
    int ret = 0;

    if (get_mmc_part_size_and_offset(partition, (unsigned int*)partSize, (unsigned int*)partOffset) == 0)
    {
        *out_size_in_bytes = (*partSize)*512;
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    return ret;
}

static int read_from_partition(const char* partition,
                               uint32_t offset,
                               size_t num_bytes,
                               void* buffer,
                               size_t* out_num_read) {
    int ret1;
    raw_io_config_push();
    ret1 = raw_io_config(FLASH_RAW_DATA_TARGET, partition, UNNECESSARY_VOLUME);

    if(ret1 != 0)
    {
        printf("raw_io_config setting fail!\n");
        return -1;
    }

    ret1 = raw_read((unsigned int)buffer, offset, num_bytes);
    if(ret1 != 0)
    {
        printf("%s:%d:%s: Error, raw_read %s fail!\n",__FILE__,__LINE__,__FUNCTION__,partition);
        return -1;
    }
    raw_io_config_pop();
    return num_bytes;

}

static int _load_dtbo(char *partition_name, uint32_t image_addr, uint32_t image_size)
{
    uint32_t ram_addr = CONFIG_DTBO_ADDR;

    if(image_addr != 0)
    {
        ram_addr = image_addr;
    }

    if(strncmp(partition_name, "boot", 4) == 0)
    {
        int ret = 0;

        uint32_t read_size = 0;
        // uint8_t pnum;
        #ifdef CONFIG_SUPPORT_VERIFY_DTBO_BY_VTS
        struct dt_table_header * hdr = (struct dt_table_header *)ram_addr;
        #else
        image_header_t *hdr = (image_header_t *)ram_addr;
        #endif

        char dtbo_partition[32] = "dtbo";

        if (get_size_of_partition(dtbo_partition, &image_size) == 0)
        {
            printf("## %s: Load dtbo image from partition: %s size=%ld \n",__FUNCTION__,dtbo_partition,image_size);

            read_size = read_from_partition(dtbo_partition, 0, image_size, ram_addr, NULL);
            if (read_size != image_size)
            {
                printf("%s:%d:%s: MMC Read %s partition error.\n",__FILE__,__LINE__,__FUNCTION__, dtbo_partition);
                return -1;
            }

            #ifdef CONFIG_SUPPORT_VERIFY_DTBO_BY_VTS
            if(be32_to_cpu(hdr->magic) != DT_TABLE_MAGIC)
            #else
            if(fdt_magic(image_get_data(hdr)) != FDT_MAGIC)
            #endif
            {
                printf("%s:%d:%s: Error, it is not a valid dtbo image.\n",__FILE__,__LINE__,__FUNCTION__);
                _dump((unsigned int)hdr, 0x100);
                printf("\n");
                return -1;
            }

            #ifdef CONFIG_SUPPORT_VERIFY_DTBO_BY_VTS
            image_size = be32_to_cpu(hdr->total_size);
            #else
            image_size = image_get_image_size(hdr);
            #endif
            if(image_size > read_size)
            {
                //read_size = emmc_read(mmc_offset, ram_addr, image_size);
                read_size = read_from_partition(dtbo_partition, 0, image_size, ram_addr, NULL);
                if (read_size != image_size)
                {
                    printf("%s:%d:%s: MMC Read %s partition error.\n",__FILE__,__LINE__,__FUNCTION__, dtbo_partition);
                    return -1;
                }
            }

            #ifndef CONFIG_SUPPORT_VERIFY_DTBO_BY_VTS
            printf("Verifying dtbo image Checksum ......");
            if (!image_check_dcrc(hdr))
            {
                printf(" Bad Data CRC !\n");
                return -1;
            }
            else
            {
                printf(" OK !\n");
            }
            #endif

            printf("## Load dtbo image done, image size: %d Byte at 0x%x!\n",image_size, ram_addr);
        }
        else
        {
            printf("%s:%d:%s: Can not find the dtbo partition: %s\n",__FILE__,__LINE__,__FUNCTION__, dtbo_partition);
            return -1;
        }
    }
    else
    {
        int magic_found = 0;
        int cnt = 2048;
        uint32_t dtbo_img_addr = 0;
        uint32_t temp_addr = 0;
        uint32_t dtbo_img_size = 0;
        uint32_t dtbo_img_offset = 0;
        uint32_t header_offset = 0;
        struct andr_img_hdr ahdr;
        read_from_partition(partition_name, 0, cnt, image_addr, NULL);
        memcpy((void *)&ahdr, (void *)image_addr, sizeof(andr_img_hdr));

        magic_found = !strncmp((char *)ahdr.magic, ANDR_BOOT_MAGIC, ANDR_BOOT_MAGIC_SIZE);        
        if (!magic_found)
        {
            printf("## %s: [%s] Android boot magic not found\n", __FUNCTION__, partition_name);
            return -1;
        }

        if(ahdr.header_version && !strncmp(partition_name,"recovery", 8))
        {
            header_offset = WITH_PADDING_SIZE(sizeof(struct andr_img_hdr), ahdr.page_size);

            image_addr += header_offset;

            dtbo_img_addr = image_addr \
                + WITH_PADDING_SIZE(ahdr.kernel_size, ahdr.page_size) \
                + WITH_PADDING_SIZE(ahdr.ramdisk_size, ahdr.page_size) \
                + WITH_PADDING_SIZE(ahdr.second_size, ahdr.page_size);

            dtbo_img_size = ahdr.recovery_dtbo_size;

            dtbo_img_offset = dtbo_img_addr - image_addr + header_offset;
            printf("Recovery dtbo offset: 0x%lx, size: %ld Bytes.\n", dtbo_img_offset, dtbo_img_size);

            // load boot.img
            cnt = WITH_PADDING_SIZE(ahdr.kernel_size, ahdr.page_size) + ahdr.ramdisk_size + 3 * ahdr.page_size;
            if (cnt > MAX_IMAGE_SIZE)
            {   //limit size of boot.img
                cnt = MAX_IMAGE_SIZE;
            }

            temp_addr = 0x25000000;
            read_from_partition(partition_name, 0, cnt, temp_addr, NULL);

            // for recovery dtbo use
            #ifdef CONFIG_SUPPORT_VERIFY_DTBO_BY_VTS
            struct dt_table_header * hdr = (struct dt_table_header *)ram_addr;
            #endif

            memcpy(ram_addr,temp_addr+dtbo_img_offset,dtbo_img_size);

            #ifdef CONFIG_SUPPORT_VERIFY_DTBO_BY_VTS
            if(be32_to_cpu(hdr->magic) != DT_TABLE_MAGIC)
            {
                printf("%s:%d:%s: Error, it is not a valid dtbo image.\n",__FILE__,__LINE__,__FUNCTION__);
                return -1;
            }
            #endif
        }
    }

    return 0;
}

static int _merge_dtbo(uint32_t dtb_addr, uint32_t dtbo_addr)
{
    ulong dtbo_image_buffer = dtbo_addr;

    struct dt_table_header *dtbo_image_hdr = (struct dt_table_header *)dtbo_image_buffer;
    struct dt_table_entry *dtbo_entry_hdr = (ulong)dtbo_image_hdr + be32_to_cpu(dtbo_image_hdr->dt_entries_offset);

    struct fdt_header *overlay_fdt = NULL;
    struct fdt_header *merged_fdt = NULL;
    ulong overlay_fdt_size = 0;
    ulong merged_fdt_size = 0;
#if defined(CONFIG_ANDROID_BOOT)
    char dtbo_index_string[64] = {0};
#endif
    uint32_t dtbo_entry_index = 0;
    uint32_t take_time = 0;

    merged_fdt = dtb_addr;
    merged_fdt_size = fdt_totalsize(dtb_addr);

    take_time = get_timestamp();
    for(dtbo_entry_index = 0; dtbo_entry_index < be32_to_cpu(dtbo_image_hdr->dt_entry_count); dtbo_entry_index++, dtbo_entry_hdr++)
    {
        overlay_fdt = (ulong)dtbo_image_hdr + be32_to_cpu(dtbo_entry_hdr->dt_offset);
        overlay_fdt_size = be32_to_cpu(dtbo_entry_hdr->dt_size);

        if(fdt_magic(overlay_fdt) != FDT_MAGIC)
        {
            printf("%s:%d:%s: Error, dtbo entry <%d> is not a valid fdt blob !\n",__FILE__,__LINE__,__FUNCTION__,dtbo_entry_index);
            return 1;
        }

        printf("   Applying the overlay fdt of index: %d ... ", dtbo_entry_index);

        merged_fdt = ufdt_apply_overlay(merged_fdt, merged_fdt_size, overlay_fdt, overlay_fdt_size);

        if (merged_fdt == NULL)
        {
            printf("%s:%d:%s: Error, get merged fdt fail!\n",__FILE__,__LINE__,__FUNCTION__);
            return 1;
        }
        else
        {
            printf("OK\n");
        }
        merged_fdt_size = fdt_totalsize(merged_fdt);

#if defined(CONFIG_ANDROID_BOOT)
        if(strlen(dtbo_index_string) == 0)
        {
            char temp_string[16] = {0};
            sprintf(temp_string, "%d", dtbo_entry_index);
            strcpy(dtbo_index_string,temp_string);
        }
        else
        {
            char temp_string[16] = {0};
            sprintf(temp_string, ",%d", dtbo_entry_index);
            strcat(dtbo_index_string,temp_string);
        }
#endif

    }
    take_time = get_timestamp() - take_time;

    //*of_flat_tree = merged_fdt;
    //*of_size = merged_fdt_size;

#if defined(CONFIG_ANDROID_BOOT)
    if(strlen(dtbo_index_string) > 0)
    {
        extern void env_add_boot_dtbo_index(char* index_string);
        env_add_boot_dtbo_index(dtbo_index_string);
    }
#endif

    memcpy(dtb_addr, merged_fdt, merged_fdt_size);
    printf("## %s: Merge DTBO to main DTB done, takes %d.%03d ms\n",__FUNCTION__,take_time/1000,take_time%1000);

    return 0;
}

int do_ufdt (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 4)
        return cmd_usage(cmdtp);

    if (strncmp(argv[1], "load", 4)==0)
    {
        uint32_t dtbo_addr = simple_strtoul(argv[3], NULL, 16);
        if (_load_dtbo(argv[2] ,dtbo_addr, 0) != 0)
        {
            printf("Load DTBO Fail\n");
            return -1;
        }
    }
    else if (strncmp(argv[1], "overlay", 7)==0)
    {
        uint32_t dtb_addr = simple_strtoul(argv[2], NULL, 16);
        uint32_t dtbo_addr = simple_strtoul(argv[3], NULL, 16);
        if (_merge_dtbo(dtb_addr, dtbo_addr) != 0)
        {
            printf("Overlay DTBO Fail\n");
            return -1;
        }
    }
    else if (strncmp(argv[1], "print", 5)==0)
    {
        uint32_t dtb_addr = simple_strtoul(argv[2], NULL, 16);
        char cmd[30] = {0};
        snprintf(cmd, 29, "fdt addr 0x%x", dtb_addr);
        if (run_command(cmd, 0) != 0)
        {
            printf("Set DTBO Addr Fail\n");
            return -1;
        }

        if (run_command("fdt print", 0) != 0)
        {
            printf("Print DTBO Fail\n");
            return -1;
        }
    }

	return 0;
}

U_BOOT_CMD(
	ufdt,	5,	0,	do_ufdt,
	"ufdt load  <dtbo_addr>                     - Load DTBO to dtbo_addr\n"
	"ufdt overlay <dtb_addr> <dtbo_addr>        - Do overlay function\n"
	"ufdt print <merge_dtb_addr>                - Print dtb data\n",
	""
);
