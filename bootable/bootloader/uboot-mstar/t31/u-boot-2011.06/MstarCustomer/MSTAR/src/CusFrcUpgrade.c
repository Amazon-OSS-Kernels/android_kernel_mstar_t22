#include <drvIIC.h>
#include <common.h>
#include <MsTypes.h>
#include <CusUpgrade.h>
#include <CusUpgradeUtility.h>
#include <MsUpgrade.h>
#include <MsUpgradeUtility.h>
#include <MsSystem.h>
#include <MsUtility.h>
#include <MsVfs.h>
#include <MsDebug.h>
#include <fat.h>
#include <MsBoot.h>
#include <MsAppCB.h>
#include <MsOS.h>
#include <apiSWI2C.h>
#include <CusCastTV.h>
#include <bootlogo/iniparser.h>
#include"mstar_string_emnu.h"
#include <ursa/ursa_common.h>


#define IIC_ADDR 0x34
#define IIC_ADDR_7827 0xa8
#define DATA_SIZE 4

#define FRC3710_TIMEOUT 1000 //wait for idle times 200 ?
#define FRC7827_TIMEOUT 1000 //wait result for frc7827

#define PAGE_SIZE 200 //200 bit
#define BIN_OFFSET 0x80000 //512K

#define FRC_OK 1
#define FRC_NOT_OK -1

#define FRC_3710 3710
#define FRC_7827 7827

#define FRC_BIN_PATH            "/FrcUpgrade.bin"

#define FRC_VERSION_CHECK "frc_version_check"

static int frc3710_unlock_status = 0;

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define GWIN_WIDTH              720
#define GWIN_HEIGHT             576
#define GRAPHIC_WIDTH           600
#define GRAPHIC_HEIGHT          400
#define GRAPHIC_X               60
#define GRAPHIC_Y               88
#define LINE_HEIGHT             50
#define RECT_LEFT_INTERVAL      50

#if (ENABLE_MODULE_DISPLAY_OSD==1)
static int display_osd = 0;
#endif

#if (((ENABLE_MODULE_USB == 1)||(ENABLE_MODULE_NETUPDATE == 1)||(ENABLE_MODULE_OAD == 1))&&(ENABLE_MODULE_DISPLAY_OSD==1))
static int show_LoadData(int var)
{
    if (-1 == display_osd)
        return -1;
    char buffer[CMD_BUF]="\0";
    int ret = 0;
    UBOOT_TRACE("IN\n");
    printf("Show upgrade  logo-->start\n");
    snprintf(buffer, CMD_BUF, "osd_create %d %d", GWIN_WIDTH, GWIN_HEIGHT);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    ret = run_command(buffer, 0);
    if (-1 == ret)
    {
        display_osd = -1;
        return -1;
    }

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 LOADING TCON DATA...", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "osd_flush");
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);
    UBOOT_TRACE("OK\n");
    return 0;
}

static int show_StartUpgrading(int var)
{
    if (-1 == display_osd)
        return -1;
    char buffer[CMD_BUF]="\0";
    UBOOT_TRACE("IN\n");

    snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 UPGRADING TCON FIRMWARE", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 PLEASE DO NOT TURN OFF", GRAPHIC_X,  GRAPHIC_Y + LINE_HEIGHT * 3);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL, GRAPHIC_Y + LINE_HEIGHT * 5, 1);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "osd_flush");
    run_command(buffer, 0);

    UBOOT_TRACE("OK\n");
    return 0;
}
static int progress_cnt = 0;
static int show_Upgrading(int current_cnt, int total_cnt)
{
    if (-1 == display_osd)
        return -1;

    char buffer[CMD_BUF]="\0";
    int current_progress = 1;
    UBOOT_TRACE("IN\n");

    current_progress = (current_cnt * 100) / total_cnt;
    if (current_progress == 0)
        current_progress = 1;
    else if (current_progress> 100)
        current_progress = 100;

    if (current_progress != progress_cnt) {
        progress_cnt = current_progress;
        memset(buffer, 0 , CMD_BUF);
        snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 5, GRAPHIC_WIDTH, LINE_HEIGHT);
        UBOOT_DEBUG("cmd=%s\n",buffer);
        run_command(buffer, 0);

        memset(buffer, 0 , CMD_BUF);
        snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL,GRAPHIC_Y + LINE_HEIGHT * 5, progress_cnt);
        UBOOT_DEBUG("cmd=%s\n",buffer);
        run_command(buffer, 0);

        memset(buffer, 0 , CMD_BUF);
        snprintf(buffer, CMD_BUF, "osd_flush");
        UBOOT_DEBUG("cmd=%s\n",buffer);
        run_command(buffer, 0);
        UBOOT_TRACE("OK\n");
    }
    return 0;
}

static int show_Error(int var)
{
    if (-1 == display_osd)
        return -1;
    char buffer[CMD_BUF]="\0";
    UBOOT_TRACE("IN\n");

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 UPGRADING TCON ERROR...", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 PLEASE IGNORE THE ERROR!!!", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 3);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);


    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "osd_flush");
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);
    UBOOT_TRACE("OK\n");
    return 0;
}

static int show_Finish(int var)
{
    if (-1 == display_osd)
        return -1;

    char buffer[CMD_BUF]="\0";
    UBOOT_TRACE("IN\n");

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 UPGRADING TCON SUCCESS...", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL, GRAPHIC_Y + LINE_HEIGHT * 5, 100);
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    memset(buffer, 0 , CMD_BUF);
    snprintf(buffer, CMD_BUF, "osd_flush");
    UBOOT_DEBUG("cmd=%s\n",buffer);
    run_command(buffer, 0);

    mdelay(100);

    UBOOT_TRACE("OK\n");
    return 0;
}
#endif

static unsigned int frc3710_swap_int32(unsigned int value) 
{
    value =
            (((value & 0x000000FF) << 24) |
            (( value & 0x0000FF00) << 8)  |
            (( value & 0x00FF0000) >> 8)  |
            (( value & 0xFF000000) >> 24));

    return value;
}

//should try unlock 5 times.
static int frc3710_unlock(void)
{
    unsigned char addr[1] = {0x80};
    unsigned char value[1] = {0x03};
    unsigned char i2cdev = 0xA6;
    unsigned char size = 0x01;
    int ret = FRC_OK;
    int try_unlock = 1;

    while (try_unlock <= 5) {
        ret = MDrv_IIC_WriteBytesPort0(i2cdev, size, addr, size, value);
        if (!ret) {
            UBOOT_ERROR("FRC3710: unloc fail, try_lock_count=%d\n", try_unlock);
            try_unlock++;
            ret = FRC_NOT_OK;
            mdelay(100);
        } else
            break;
    }

    mdelay(20);
    frc3710_unlock_status = 1;
    //callback read unlock value, double check unlock status.
    value[0] = 0x00;
    ret = MDrv_IIC_ReadBytesPort0(i2cdev, size, addr, size, value);
    if (!ret) {
        printf("[ERROR]:FRC3710 read unlock info failed: ret=%d\n", ret);
        ret = FRC_NOT_OK;
        return ret;
    }

    if (value[0] == 0x3)
        printf("unlock FRC3710 sccuss\n");
    else
        printf("unlock FRC3710 failed\n");

    return ret;
}

static int get_frc3710_unlock_status(void)
{
    if (frc3710_unlock_status == 0) {
        if (frc3710_unlock() != FRC_OK) {
            UBOOT_ERROR("FRC3710: unlock frc3710 fail\n");
            return FRC_NOT_OK;
        }
    }

    return FRC_OK;
}

//the function should be set in frc3710 init.
static int get_frc3710_version(int *version)
{
    unsigned char pu8Addr[DATA_SIZE] = {0xC0, 0x00, 0x47, 0x00};
    /*cmd_idx << 16 + parameters_number*/
    unsigned char pu8wbuf[DATA_SIZE] = {0x03, 0x80, 0x00, 0x03};
    unsigned char version_info[12];
    unsigned int *pBuf = NULL;
    int i = 0;
    int ret = FRC_OK;

    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR, 4, pu8Addr, 4, pu8wbuf);
    if (!ret) {
        printf("[FRC3710-ERROR-VERSION]: set cmd_idx and parameter\n");
        ret = FRC_NOT_OK;
        return ret;
    }

    //must delay,or get version info is 0.0.0
    mdelay(50);
    pu8Addr[3] = 0x04;
    for (i = 0; i < 3; i++) {
        pu8Addr[3] += (unsigned char)(4 * i);
        ret = MDrv_IIC_ReadBytesPort0(IIC_ADDR + 1, DATA_SIZE, pu8Addr, DATA_SIZE, version_info + (4 * i));
        if (!ret) {
            printf("get FRC3710 version info failed:i=%d,ret=%d\n", i, ret);
            ret = FRC_NOT_OK;
            return ret;
        }
    }

    pBuf = (unsigned int *)version_info;
    for ( i = 0; i < 3; i++)
        pBuf[i] = frc3710_swap_int32(pBuf[i]);

    printf("Get current FRC3710 version info:%u.%u.%u\n",
            pBuf[0], pBuf[1], pBuf[2]);

    //only need save SVN info to save version info.
    *version = pBuf[0];

    return ret;
}

static int set_frc3710_video_mode(void)
{
    int ret = FRC_OK;
    unsigned char addr[4] = {0xC0,0x00,0x47,0x04};
    unsigned char value[4] = {0x00,0x00,0x00,0x00};
    unsigned char set_cmd_idx[4] = {0x01,0x00,0x00,0x00};

    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR, 4, addr, 4, value);
    if (!ret) {
        UBOOT_ERROR("FRC3710: set video mode write fail\n");
        return FRC_NOT_OK;
    }

    addr[3] = 0x00;
    set_cmd_idx[3] = 0x1;
    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR, 4, addr, 4, set_cmd_idx);
    if (!ret) {
        UBOOT_ERROR("FRC3710: set video read fail\n");
        return FRC_NOT_OK;
    }

    printf("FRC3710: set video mode success\n");
    return ret;
}

static void U32_To_U8(unsigned char *arry, int size, unsigned int value)
{
    int i = 0;

    if (0 == size || NULL == arry) {
        UBOOT_ERROR("array is NULL or size is zero! U32_To_U8\n");
        return;
    }

    for (i = 0; i < size; i++) {
        int offset = 8 * (size - i - 1);
        arry[i] = (unsigned char)((value >> offset) & 0xFF);
    }
}

static int write_frc3710_register(unsigned int addr, unsigned int value)
{
    unsigned char pu8Value[DATA_SIZE] = {0x00,0x00,0x00,0x00};
    unsigned char pu8Addr[DATA_SIZE] = {0x00,0x00,0x00,0x00};
    int ret = FRC_OK;

    U32_To_U8(pu8Addr, DATA_SIZE, addr);
    U32_To_U8(pu8Value, DATA_SIZE, value);

    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR, DATA_SIZE, pu8Addr, DATA_SIZE, pu8Value);
    if (!ret) {
        UBOOT_ERROR("FRC3710:read register i2c fail\n");
        ret = FRC_NOT_OK;
    }

    return ret;
}

static int send_erase_cmd_frc3710(unsigned int size)
{
    int ret = FRC_OK;

    if (write_frc3710_register(0xc0004718, 0xaaaa0000) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0x4718-0xaaaa0000 fail\n");
        return FRC_NOT_OK;
    }
    mdelay(20);

    if (write_frc3710_register(0xc0004710, size) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0x4710-0x%x fail\n", size);
        return FRC_NOT_OK;
    }
    mdelay(20);

    if (write_frc3710_register(0xc0004704, 1) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0x4704-0x01 fail\n");
        return FRC_NOT_OK;
    }
    mdelay(20);

    if (write_frc3710_register(0xc0004700, 0x0b000000) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0x4700-0x0b000000 fail\n");
        return FRC_NOT_OK;
    }

    return ret;
}

static int frc_send_write_cmd(unsigned int uiPageIndex)
{
    int ret = FRC_OK;

    if (get_frc3710_unlock_status() != FRC_OK) {
        UBOOT_ERROR("FRC3710: frc unlock fail: write register\n");
	    return FRC_NOT_OK;
    }

    if (write_frc3710_register(0xc0004718, 0xaaaa0000) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0xc0004718-0xaaaa0000 fail\n");
        return FRC_NOT_OK;
    }

    if (write_frc3710_register(0xc0004708, uiPageIndex) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0xc0004708,uiPageIndex=%u\n", uiPageIndex);
        return FRC_NOT_OK;
    }

    if (write_frc3710_register(0xc0004704, 0x02) != FRC_OK) {
        UBOOT_ERROR("frc3710: write register 0xc0004704-0x02 fail\n");
        return FRC_NOT_OK;
    }

    if (write_frc3710_register(0xc0004700, 0x0b000000) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write register 0xc0004700-0x0b000000 fail\n");
        return FRC_NOT_OK;
    }

    return ret;
}

static int wait_for_idle(void)
{
    unsigned int u32Tmp_1 = 0;
    unsigned char addr[DATA_SIZE] = {0xc0, 0x00, 0x47, 0x18};
    unsigned char value[DATA_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00};
    int ret = FRC_OK;
    int whileCount = 0;

    if (get_frc3710_unlock_status() != FRC_OK) {
        UBOOT_ERROR("FRC3710: wait_for_idle get unlcok status faul\n");
        return FRC_NOT_OK;
    }

    while (1) {
        ret = MDrv_IIC_ReadBytesPort0(IIC_ADDR + 1, 4, addr, 4, value);
        u32Tmp_1 |= (value[0] << 24);
        u32Tmp_1 |= (value[1] << 16);
        u32Tmp_1 |= (value[2] << 8);
        u32Tmp_1 |= (value[3] << 0);

        if (!ret) {
            UBOOT_ERROR("FRC3710: wait_for_idle read fail\n");
        }
        if (u32Tmp_1 == 0xaaaa5555) {
            ret = FRC_OK;
            break;
        } else {
            mdelay(20);
            whileCount++;
        }

        if (whileCount == FRC3710_TIMEOUT) {
            UBOOT_ERROR("FRC3710: wait_for_idle timeout: wanted value=0x%x, but value=0x%x\n",
                       0xaaaa5555, u32Tmp_1);
            ret = FRC_NOT_OK;
            break;
        }
    }

    return ret;
}

static int frc_read_checksum(unsigned int *checksum)
{
    unsigned int iTemp = 0;
    int ret = FRC_OK;
    unsigned char addr[DATA_SIZE] = {0xc0, 0x00, 0x47, 0x1c};
    unsigned char value[DATA_SIZE] = {0x00, 0x00, 0x00, 0x00};


    //get unlock status
    if (get_frc3710_unlock_status() != FRC_OK) {
        UBOOT_ERROR("FRC3710: unlock fail:frc_read_checksum\n");
        ret = FRC_NOT_OK;
        return ret;
    }

    ret = MDrv_IIC_ReadBytesPort0(IIC_ADDR + 1, 4, addr, 4, value);
    if (!ret) {
        UBOOT_ERROR("FRC3710: read checksum fail\n");
        ret = FRC_NOT_OK;
        return ret;
    }
    iTemp |= (value[0] << 24);
    iTemp |= (value[1] << 16);
    iTemp |= (value[2] << 8);
    iTemp |= (value[3] << 0);

    *checksum = iTemp;
    return ret;
}


static int frc_send_flash_cmd(void)
{
    int ret = FRC_OK;

    //get unlock status
    if (get_frc3710_unlock_status() != FRC_OK) {
        UBOOT_ERROR("FRC3710: unlock fail:frc_send_flash_cmd\n");
        return FRC_NOT_OK;
    }

    if (write_frc3710_register(0xc0004700, 0x0b000000) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write 0xc0004700 fail\n");
        return FRC_NOT_OK;
    }

    if (write_frc3710_register(0xc0004704, (unsigned int)0x03) != FRC_OK) {
        UBOOT_ERROR("FRC3710: write 0xc0004704-0x03 fail\n");
        return FRC_NOT_OK;
    }

    return ret;
}

static int update_frc3710_fw(unsigned char *fp, unsigned int size)
{
    unsigned int iLength;
    unsigned int iPages;
    unsigned int iLeft;
    unsigned int uiChecksum = 0;
    unsigned int uiReadChecksum = 0xFF;
    unsigned int n, i, j;
    unsigned int iTempWriteData = 0;
    int ret = FRC_OK;
    int watchdog_disable_status = 0;

    if (MDrv_WDT_IsEnable()) {
        run_command("wdt_enable 0", 0);
        watchdog_disable_status = 1;
        printf("frc3710: disable watchdog\n");
    }

    if (!fp || !size) {
        UBOOT_ERROR("FRC3710: fw pointer or size is NULL\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    //1. get fw actual address and size info.
    fp += BIN_OFFSET; //offset 512K
    iLength = size - BIN_OFFSET;
    iPages = iLength / PAGE_SIZE;
    iLeft = iLength % PAGE_SIZE;
    printf("FRC3710 start upgrade fw info: iLength=0x%x, iPage=0x%x, iLeft=0x%x\n",
            iLength, iPages, iLeft);

    //update progress -1
    show_LoadData(0);

    //2. send erase command
    ret = send_erase_cmd_frc3710(iLength);
    if (ret != FRC_OK) {
        UBOOT_ERROR("FRC3710: first---send_erase_cmd_frc3710 fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }
    printf("FRC3710: erase success\n");
    //if not add delay will result wait_for_idle fail or while count is huge count.
    mdelay(2000);
    //3. wait for idle
    ret = wait_for_idle();
    if (ret != FRC_OK) {
        UBOOT_ERROR("FRC3710: wait for idle faile\n");
        goto end_func;
    }
    show_StartUpgrading(0);
    //4. start upgrade
    j = PAGE_SIZE / 4;
    for (i = 0; i < iPages; i++) {
        uiChecksum = 0;
        uiReadChecksum = 0xFF;
        while (uiReadChecksum != uiChecksum) {
            uiChecksum = 0;
            uiReadChecksum = 0xFF;

            for (n = 0; n < j; n++) {
                iTempWriteData = 0;
                iTempWriteData |= (unsigned int)(fp[200 * i + n * 4 + 0] << (3 * 8));
                iTempWriteData |= (unsigned int)(fp[200 * i + n * 4 + 1] << (2 * 8));
                iTempWriteData |= (unsigned int)(fp[200 * i + n * 4 + 2] << (1 * 8));
                iTempWriteData |= (unsigned int)(fp[200 * i + n * 4 + 3] << (0 * 8));

                ret = write_frc3710_register(0xc000472c + n * 4, iTempWriteData);
                if (ret != FRC_OK) {
                    UBOOT_ERROR("FRC3710: wirte 0x%x fail, iTempWriteData=0x%x\n", 0xc000472c+n*4, iTempWriteData);
                    ret = FRC_NOT_OK;
                    goto end_func;
                }

                uiChecksum = uiChecksum +
                             fp[200 * i + n * 4 + 0] +
                             fp[200 * i + n * 4 + 1] +
                             fp[200 * i + n * 4 + 2] +
                             fp[200 * i + n * 4 + 3];
            }//first for end

            if (frc_send_write_cmd(i) != FRC_OK) {
                UBOOT_ERROR("FRC3710: frc send write cmd fail %u\n", i);
                ret = FRC_NOT_OK;
                goto end_func;
            }

            if (wait_for_idle() != FRC_OK) {
                UBOOT_ERROR("FRC3710: wait for idle fail\n");
                ret = FRC_NOT_OK;
                goto end_func;
            }

            if (frc_read_checksum(&uiReadChecksum) != FRC_OK) {
                UBOOT_ERROR("FRC3710: read checksum fail\n");
                ret = FRC_NOT_OK;
                goto end_func;
            }
        }//while end
        show_Upgrading(i, iPages);
        mdelay(10);
    }//for end

    if (wait_for_idle() != FRC_OK) {
        UBOOT_ERROR("FRC3710: wait_for_idle fail i =%u\n", i);
        ret = FRC_NOT_OK;
        goto end_func;
    }
    mdelay(100);
    if (iLeft > 0) {
        j = iLeft / 4;
        for (n = 0; n < j; n++) {
            int temp = 0;

            temp = 200 * i + n * 4;
            iTempWriteData = 0;
            iTempWriteData |= (unsigned int)(fp[temp + 0] << (3 * 8));
            iTempWriteData |= (unsigned int)(fp[temp + 1] << (2 * 8));
            iTempWriteData |= (unsigned int)(fp[temp + 2] << (1 * 8));
            iTempWriteData |= (unsigned int)(fp[temp + 3] << (0 * 8));

            ret = write_frc3710_register(0xc000472c +n*4, iTempWriteData);
            if (ret != FRC_OK) {
                UBOOT_ERROR("FRC3710: wirte 0x%x fail, iTempWriteData=0x%x\n", 0xc000472c+n*4, iTempWriteData);
                ret = FRC_NOT_OK;
                goto end_func;
            }
        }
    }

    //update progress -3
    show_Upgrading(iPages, iPages);

    mdelay(200);
    if (frc_send_write_cmd(i) != FRC_OK) {
        UBOOT_ERROR("FRC3710: frc send write cmd fail %u\n", i);
        ret = FRC_NOT_OK;
        goto end_func;
    }

    mdelay(200);
    if (wait_for_idle() != FRC_OK) {
        UBOOT_ERROR("FRC3710: wait for idle fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    mdelay(200);
    if (frc_send_flash_cmd() != FRC_OK) {
        UBOOT_ERROR("FRC3710: frc_send_flash_cmd fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    mdelay(200);
    if (wait_for_idle() != FRC_OK) {
        UBOOT_ERROR("FRC3710: wait for idle fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    //update progress -4
    show_Finish(0);
    mdelay(3000);

end_func:
    if (watchdog_disable_status == 1) {
        printf("frc3710: enable watchdog\n");
        run_command("wdt_enable", 0);
    }
    return ret;
}

static frc7827_write_register(unsigned int addr, unsigned int value)
{
    unsigned char pu8Value[DATA_SIZE] = {0x00,0x00,0x00,0x00};
    unsigned char pu8Addr[DATA_SIZE] = {0x00,0x00,0x00,0x00};
    int ret = FRC_OK;

    U32_To_U8(pu8Addr, DATA_SIZE, addr);
    U32_To_U8(pu8Value, DATA_SIZE, value);

    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR_7827, DATA_SIZE, pu8Addr, DATA_SIZE, pu8Value);
    if (!ret) {
        UBOOT_ERROR("FRC:write register i2c fail addr=0x%x,value=0x%x\n", addr, value);
        ret = FRC_NOT_OK;
    }

    return ret;
}

static int frc7827_read_register(unsigned int addr, unsigned int *value)
{
    unsigned char pu8Value[DATA_SIZE] = {0x00,0x00,0x00,0x00};
    unsigned char pu8Addr[DATA_SIZE] = {0x00,0x00,0x00,0x00};
    int ret = FRC_OK;
    unsigned int iTemp = 0;

    U32_To_U8(pu8Addr, DATA_SIZE, addr);

    ret = MDrv_IIC_ReadBytesPort0(IIC_ADDR_7827 + 1, DATA_SIZE, pu8Addr, DATA_SIZE, pu8Value);
    if (!ret) {
        UBOOT_ERROR("FRC:read register i2c fail addr=0x%x,value=0x%x\n", addr, value);
        ret = FRC_NOT_OK;
        return ret;
    }
    iTemp |= (pu8Value[0] << 24);
    iTemp |= (pu8Value[1] << 16);
    iTemp |= (pu8Value[2] << 8);
    iTemp |= (pu8Value[3] << 0);

    *value = iTemp;

    return ret;
}

static int wait_for_result(unsigned int addr)
{
    int whileCount = 0;
    unsigned int uiStatus = 0;
    unsigned int uiCommand = 0;
    unsigned char pu8Addr[DATA_SIZE] = {0x00,0x00,0x00,0x00};

    while (1) {
        if (frc7827_read_register(addr, &uiCommand) != FRC_OK) {
            UBOOT_ERROR("frc7827: read addr=0x%x fail\n", addr);
            return FRC_NOT_OK;
        };

        mdelay(10);
        if (frc7827_read_register(addr + 0x28, &uiStatus) != FRC_OK) {
            UBOOT_ERROR("frc7827: read addr=0x%x fail\n", addr + 0x28);
            return FRC_NOT_OK;
        }

        uiStatus &= 0xFF;

        if (uiStatus == 1 && uiCommand == 0xFFFFFFFF) {
            return FRC_OK;
        }

        whileCount++;
        if (whileCount >= FRC7827_TIMEOUT) {
            UBOOT_ERROR("frc7827: wait for result, want value 0xFFFFFFFF ?=0x%x, want status 1?=%u\n",
                        uiCommand, uiStatus);
            return FRC_NOT_OK;
        }
    }
}

static int get_frc7827_version(int *version)
{
    unsigned char pu8Addr[DATA_SIZE] = {0xc0, 0x00, 0x9b, 0x00};
    unsigned char pu8Value[DATA_SIZE] = {0x77, 0x00, 0x00, 0x00};
    int ret = FRC_OK;
    int currVersion = 0;

    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR_7827, DATA_SIZE, pu8Addr, DATA_SIZE, pu8Value);
    if (!ret) {
        UBOOT_ERROR("frc7827: get version write fail\n");
        ret = FRC_NOT_OK;
    }

    pu8Addr[3] = 0x04;
    pu8Value[0] = 0x00;
    mdelay(50);
    ret = MDrv_IIC_ReadBytesPort0(IIC_ADDR_7827 + 1, DATA_SIZE, pu8Addr, DATA_SIZE, pu8Value);
    if (!ret) {
        UBOOT_ERROR("frc7827: get version read fail\n");
        ret = FRC_NOT_OK;
    }
    currVersion |= pu8Value[0] << 24;
    currVersion |= pu8Value[1] << 16;
    currVersion |= pu8Value[2] << 8;
    currVersion |= pu8Value[3];

    *version = currVersion;
    printf("frc7827: version = %d\n", currVersion);

    return ret;
}

static int set_frc7827_video_mode(void)
{
    int ret = FRC_OK;
    unsigned char pu8Addr[DATA_SIZE] = {0xc0, 0x00, 0x9b, 0x00};
    unsigned char pu8Value[DATA_SIZE] = {0x00, 0x00, 0x00, 0x00};
    unsigned char cmd_idx = 0x08;

    pu8Value[0] = cmd_idx;//pu8Value[3] = 0x00 is meaning to set video mode.

    ret = MDrv_IIC_WriteBytesPort0(IIC_ADDR_7827, DATA_SIZE, pu8Addr, DATA_SIZE, pu8Value);
    if (!ret) {
        UBOOT_ERROR("frc7827: set video mode fail\n");
        ret = FRC_NOT_OK;
    }
    printf("FRC7827: set video mode success\n");

    return ret;
}

static int update_frc7827_fw(unsigned char *fp, unsigned int size)
{
    unsigned int lenght = 0;
    unsigned int uiCalcCheckSum = 0;
    unsigned int uiReadCheckSum = 0;
    unsigned int uiCCPRegAddr = 0xc0009b00;
    unsigned int uiCCPDataAddr = 0;
    unsigned int i;
    unsigned int iTempWriteData = 0;
    int ret = FRC_OK;
    unsigned int offset_size = 0;
    unsigned int upgrade_size = 0;
    unsigned char *upgrade_data = NULL;
    unsigned int *upgrade_int_data = NULL;
    int watchdog_disable_status = 0;

    if (MDrv_WDT_IsEnable()) {
        run_command("wdt_enable 0", 0);
        watchdog_disable_status = 1;
    }

    if (!fp || !size) {
        UBOOT_ERROR("FRC7827: fw pointer or size is NULL\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (size < 0x40000) {
        UBOOT_ERROR("frc7827: wrong bin file\n");
        ret = FRC_NOT_OK;
        goto end_func;
    } else if (size < 0x80000)
        offset_size = 0x40000;
    else if (size < 0x100000)
        offset_size = 0x80000;
    else if (size < 0x200000)
        offset_size = 0x100000;
    else
        offset_size = 0x80000;

    upgrade_data = (unsigned char *)malloc((size - offset_size) + 4);
    if (upgrade_data == NULL) {
        UBOOT_ERROR("frc7827: malloc upgrade data is NULL\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }
    upgrade_size = size - offset_size;

    printf("frc7827: size=0x%x, offset_size=0x%x, upgrade_size=0x%x ?= 0x%x\n", size, offset_size, upgrade_size, size - offset_size);

    for (i = 0; i < upgrade_size; i++) {
        upgrade_data[i] = fp[i + offset_size];
        uiCalcCheckSum = uiCalcCheckSum + upgrade_data[i];
    }

    upgrade_int_data = (unsigned int *)malloc((size - offset_size) + 4);
    if (upgrade_int_data == NULL) {
        UBOOT_ERROR("frc7827: malloc upgrade int data is NULL\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }
    show_LoadData(0);
    mdelay(100);
    show_StartUpgrading(0);
    //1. sent CMD_UPGRADE_START and get data address
    if (frc7827_write_register(uiCCPRegAddr, 0x60000000) != FRC_OK) {
        UBOOT_ERROR("frc7827: sent CMD_UPGRADE_START write fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (wait_for_result(uiCCPRegAddr) != FRC_OK) {
        UBOOT_ERROR("frc7827: sent CMD_UPGRADE_START wait fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (frc7827_read_register(uiCCPRegAddr + 4, &uiCCPDataAddr) != FRC_OK) {
        UBOOT_ERROR("frc7827: sent CMD_UPGRADE_START read fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }
    printf("frc7827:1 Send Upgrade start Cmd and Get data address... uiCCPDataAddr=0x%x\n", uiCCPDataAddr);

    /*2.send: CMD_UPGRADE_TRANSMIT_DATA to send the data size and dual
      image offset and write data to the data address*/
    if (frc7827_write_register(uiCCPRegAddr + 4, upgrade_size) != FRC_OK) {
        UBOOT_ERROR("frc7827: send the data size fail\n");
        goto end_func;
    }

    if (frc7827_write_register(uiCCPRegAddr + 8, offset_size) != FRC_OK) {
        UBOOT_ERROR("frc7827: send dual image offset fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (frc7827_write_register(uiCCPRegAddr, 0xa0000002) != FRC_OK) {
        UBOOT_ERROR("frc7827: write data address 0xa000002 fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (wait_for_result(uiCCPRegAddr) != FRC_OK) {
        UBOOT_ERROR("frc7827: CMD_UPGRADE_TRANSMIT_DATA wait result fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    lenght = upgrade_size / 4;
    for (i = 0; i < lenght; i++) {
        unsigned int temp = i * 4;

        upgrade_int_data[i]  =  upgrade_data[temp]     * 0x1000000 +
                                upgrade_data[temp + 1] * 0x10000 +
                                upgrade_data[temp + 2] * 0x100 +
                                upgrade_data[temp + 3];

        if (frc7827_write_register(uiCCPDataAddr + temp, (unsigned int)upgrade_int_data[i]) != FRC_OK) {
            UBOOT_ERROR("frc7827: i=%d,write addr=0x%x value=0x%x fail\n", i, uiCCPRegAddr + temp, upgrade_int_data[i]);
            ret = FRC_NOT_OK;
            goto end_func;
        }

        show_Upgrading(i, lenght);
    }
    printf("frc7827:2 Writing data is complete... \n");

    /*3. Send: CMD_UPGRADE_CHECKSUM to get the data checksum and compare,
      if not match, repeat step 2 and 3.*/
    if (frc7827_write_register(uiCCPRegAddr, 0x61000000) != FRC_OK) {
        UBOOT_ERROR("frc7827: write 0x61000000 fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (wait_for_result(uiCCPRegAddr) != FRC_OK) {
        UBOOT_ERROR("frc7827: wait CMD_UPGRADE_CHECKSUM result fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }
    if (frc7827_read_register(uiCCPRegAddr + 4, &uiReadCheckSum) != FRC_OK) {
        UBOOT_ERROR("frc7827: read uiReadCheckSum fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }
    if (uiReadCheckSum != uiCalcCheckSum) {
        UBOOT_ERROR("frc7827: 3 Get checksum is not match, should retry, uiReadCheckSum=0x%x, uiCalcCheckSum=0x%x\n",
                    uiReadCheckSum, uiCalcCheckSum);
        ret = FRC_NOT_OK;
        goto end_func;
    } else
        printf("frc7827: 3 checkSum match success....\n");

    if (frc7827_write_register(uiCCPRegAddr, 0x21000000) != FRC_OK) {
        UBOOT_ERROR("frc7827: write 0x21000000 fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    if (wait_for_result(uiCCPRegAddr) != FRC_OK) {
        UBOOT_ERROR("frc7827: wait upgrade completely result fail\n");
        ret = FRC_NOT_OK;
        goto end_func;
    }

    show_Upgrading(lenght, lenght);
    printf("frc7827: 4 Upgrade completely\n");

    show_Finish(0);
    mdelay(3000);

end_func:
    if (upgrade_data != NULL)
        free(upgrade_data);

    if (upgrade_int_data != NULL)
        free(upgrade_int_data);

    if (watchdog_disable_status == 1)
        run_command("wdt_enable", 0);
    return ret;
}

static int get_frc_type_version(int *type, int *version)
{
    char *s_tmp;
    char *fp = NULL;
    int fileSize = 0;
    int frc_type = 0;
    int frc_version = 0;
    char m_name[100] = "\0";
    int ret = FRC_NOT_OK;

    //From model_name ini file, we can get external frc hardware id
    idme_get_var_external("model_name", m_name, 99);
    s_tmp = m_name;

    if (s_tmp != NULL) {
        fp = (char *)loadscript(s_tmp, &fileSize);
        if (fp == NULL) {
            if (vfs_mount("tvconfig") != 0) {
                UBOOT_ERROR("mount tvconfig fail\n");
                return FRC_NOT_OK;
            }
            fp = (char *)loadscript(s_tmp, &fileSize);
        }
    }

    if (fp != NULL) {
        Profile_Init((char *)fp, fileSize);

        frc_type = Profile_GetInteger("customized_frc", "customized_frc_type", 0);
        frc_version = Profile_GetInteger("customized_frc", "customized_frc_version", 0);
        ret = FRC_OK;
    } else
        UBOOT_ERROR("can't load %s\n", s_tmp);

    *type = frc_type;
    *version = frc_version;

    return ret;
}

static int frc3710_pre_work(int *current_version)
{
    int currentVersion = 0;

//    mdelay(400);//wait power on
    //first must unlock 3710
    if (frc3710_unlock() != FRC_OK) {
        UBOOT_ERROR("FRC3710: unlock fail\n");
        return FRC_NOT_OK;
    }

    //get current using version
    if (get_frc3710_version(&currentVersion) != FRC_OK) {
        UBOOT_ERROR("FRC3710: get current version fail\n");
        return FRC_NOT_OK;
    }

    *current_version = currentVersion;
    return FRC_OK;
}

int do_frc_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = FRC_OK;
    char *fp = NULL;
    char *version = NULL;
    unsigned char *filepoiner = NULL;
    unsigned int fileSize = 0;
    int current_version = 0;
    char path[128] = "\0";
    char version_info[20] = "\0";
    char *s_tmp;
    int version_id = 0;
    int hardware_id = 0;
    unsigned long frc_upgrade_start_time = MsSystemGetBootTime();
    unsigned long frc_upgrade_duration_time = 0;
    int enable_ver_check = 0;
    char *s = NULL;

    if (get_frc_type_version(&hardware_id, &version_id) != FRC_OK || !hardware_id) {
        ret = FRC_NOT_OK;
        goto err;
    }

    /* HWIIC init. */
    MDrv_IIC_InitPort0(E_HWI2C_PORT0_3);

    if (hardware_id == FRC_3710) {
        if (frc3710_pre_work(&current_version) != FRC_OK) {
            UBOOT_ERROR("frc3710 per work fail\n");
            ret = FRC_NOT_OK;
            goto err;
        }
    } else if (hardware_id == FRC_7827) {
        if (get_frc7827_version(&current_version) != FRC_OK) {
            UBOOT_ERROR("frc7827: get version fail\n");
            ret = FRC_NOT_OK;
            goto err;
        }
    }

    //in uboot, PC mode switch to video mode in here when OTA disable
    if (!version_id)
        goto set_video_mode;

    //when need OTA, need get binary file path
    if (!Profile_GetString("customized_frc", "customized_frc_bin_path", "", path,sizearray(path))) {
        ret = FRC_NOT_OK;
        goto err;
    }

    //when OTA and USB is exist, need check USB upgrade flag.
    s = getenv(FRC_VERSION_CHECK);
    if (NULL != s) {
        enable_ver_check = (int) simple_strtoul (s, NULL, 10);
        if (enable_ver_check == 1) {
            UBOOT_ERROR("ignore frc OTA upgrade\n");
            goto set_video_mode;
        }
        printf("frc enable_ver_check=%d\n", enable_ver_check);
    } else {
        UBOOT_ERROR("FRC_VERSION_CHECK is NULL, normal OTA!\n");
    }

    if (current_version != version_id) {
        printf("version is not the same, should upgrade frc firmware current version=%d, need upgrade version=%d\n",
               current_version, version_id);
    } else {
        UBOOT_ERROR("version is the same:current verison id=%d, model_name fw version id=%d, should not upgrade frc firmware\n",
                   current_version, version_id);
        ret = FRC_NOT_OK;
        goto set_video_mode;
    }

    s_tmp = path;
    //must remove partition name.
    while (*s_tmp == '/') s_tmp++;
    s_tmp = strstr(s_tmp,"/");
    printf("FRC: fw file name:%s\n", s_tmp);

    fp = (char *)loadscript(s_tmp, &fileSize);
    if (NULL == fp) {
        UBOOT_ERROR("FRC: load frc fw file fail:%s\n", path);
        ret = FRC_NOT_OK;
        goto err;
    }

    run_command("osd_destroy", 0);
    printf("FRC: load fw to dram success and start upgrade~\n");
    filepoiner = (unsigned char*)fp;
    //4. different path
    if (hardware_id == FRC_3710 && update_frc3710_fw(filepoiner, fileSize) != FRC_OK) {
        UBOOT_ERROR("FRC3710: Burning fw fail\n");
        ret = FRC_NOT_OK;
        goto err;
    } else if(hardware_id == FRC_7827 && update_frc7827_fw(filepoiner, fileSize) != FRC_OK) {
        UBOOT_ERROR("FRC7827: Burning fw fail\n");
        ret = FRC_NOT_OK;
        goto err;
    }

    setenv(FRC_VERSION_CHECK, "0");//enable version check.
    saveenv();
    printf("\n*********** Burning FW success ***********\n");

set_video_mode:
    if (hardware_id == FRC_3710 && set_frc3710_video_mode() != FRC_OK) {
        UBOOT_ERROR("FRC3710: set video mode fail\n");
        ret = FRC_NOT_OK;
    } else if (hardware_id == FRC_7827 && set_frc7827_video_mode() != FRC_OK) {
        UBOOT_ERROR("FRC7827: set video mode fail\n");
        ret = FRC_NOT_OK;
    }
    frc_upgrade_duration_time = MsSystemGetBootTime() - frc_upgrade_start_time;
    printf("external frc_upgrade time is %lu ms \n", frc_upgrade_duration_time);
    return 0;
err:
    return -1;
}

int do_frc_usb_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cmd[CMD_BUF] = "\0";
    char *env =NULL;
    int partition = 0;
    int device = 0;
    int ret = 0;
    MS_U8 u8Percent= 0;
    long unsigned  int u32FileSize = 0;
    int version_id = 0;
    int hardware_id = 0;
    unsigned char *fp = NULL;

    if (get_frc_type_version(&hardware_id, &version_id) != FRC_OK || !hardware_id) {
        UBOOT_ERROR("frc: get hardware_id info fail or is not external frc module\n");
        return -1;
    }

    if (-1 == appInitUsbDisk()) {
        UBOOT_ERROR("FAIL : can not init usb!! \n");
        return -1;
    }

    ret = check_file_partition(FRC_BIN_PATH, &device, &partition);
    if(ret == 0) {
            UBOOT_DEBUG("has external frc upgrade bin\n");
            UBOOT_DEBUG("Geting file size\n");
            snprintf(cmd, CMD_BUF, "fatfilesize usb %d:%d %s", device,partition, FRC_BIN_PATH);
            UBOOT_DEBUG("cmd=%s\n",cmd);
            ret = run_command(cmd, 0);
            if(ret!=0) {
                UBOOT_ERROR("get script file's size fail\n");
                return -1;
            } else {
                env=getenv("filesize");
                if(env==NULL)
                   UBOOT_ERROR("get env 'filesize' fail\n");
                u32FileSize = simple_strtol(env, NULL, 16);
                printf("Size 0x%lx \n",u32FileSize);
           }

            snprintf(cmd, CMD_BUF, "fatload usb %d:%d  %X %s %ul",device, partition, UPGRADE_BUFFER_ADDR, FRC_BIN_PATH,u32FileSize);

            UBOOT_DEBUG("cmd=%s\n",cmd);
            if(-1 == run_command(cmd, 0)) {
                UBOOT_ERROR("Load Upgrade File fail!\n");
                return -1;
            } else {
                printf("Start Upgrade external frc firmware!~\n");
                run_command("panel_pre_init", 0);//panel power on
                mdelay(1000);
                /* HWIIC init. */
                MDrv_IIC_InitPort0(E_HWI2C_PORT0_3);
                fp = (unsigned char *)UPGRADE_BUFFER_ADDR;
                switch (hardware_id) {
                    case FRC_3710:
                        if (frc3710_unlock() != FRC_OK) {
                            UBOOT_ERROR("frc unlock fail\n");
                            return -1;
                        }
                        if (update_frc3710_fw(fp, u32FileSize) != FRC_OK) {
                            UBOOT_ERROR("FRC3710: Burning fw fail\n");
                            return -1;
                        } else {
                            printf("\n*********** Burning FW success ***********\n");
                            setenv(FRC_VERSION_CHECK, "1"); //1:disable version check,0:enable version check.
                            saveenv();
                            return 0;
                        }
                    case FRC_7827:
                        if (update_frc7827_fw(fp, u32FileSize) != FRC_OK) {
                            UBOOT_ERROR("FRC7827: Burning fw fail\n");
                            return -1;
                        } else {
                            printf("\n*********** Burning FW success ***********\n");
                            setenv(FRC_VERSION_CHECK, "1");
                            saveenv();
                            return 0;
                        }
                    default:
                        UBOOT_ERROR("the product is not include external frc module\n");
                        return -1;
                }
          }
    } else
        UBOOT_DEBUG("no %s in usb disk\n",FRC_BIN_PATH);
    return -1;
}
