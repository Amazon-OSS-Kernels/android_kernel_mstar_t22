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
#include "drvHWI2C.h"


#define BLU_OK 1
#define BLU_NOT_OK -1

#define BLU_FW_SLAVEADDR 0x88
#define BLU_SLAVEADDR 0x46

#define FW_UPDATE 0x90
#define ERASE_APP 0x06
#define WRITE_PM 0x02
#define GO_RESET 0x04
#define BOOT_RETRY 0x07
#define BANKSIZE 0x10000
#define BLOCKNUM 0xff
#define BLOCKSIZE 0x100
#define BUFFER_SIZE 128

#define COMMANDNUM 129

#define BLU_BIN_PATH            "/blu.bin"

#define MAX_UPGRADE_IMAGE_FILE_SIZE 64 * 1024 * 1024

typedef enum
{
    BluFlashEraseErr,
    BluFlashBlankingErr,
    BluFlashProgOK,
} BluFlashProgErrorType;

typedef enum
{
    BluFlashProgStateInit,
    BluFlashProgStateErase,
    BluFlashProgStateProgram,
    BluFlashProgStateReset,
    BluFlashProgStateExit,
    BluFlashProgStateIdle
} BluFlashProgStateType;

static unsigned char *BluFileBuf = NULL;
static int blankNumber = 0;
static unsigned char currentPercent = 0;
BluFlashProgStateType BluUpgradeState = BluFlashProgStateIdle;
BluFlashProgErrorType BluErrorFlag = BluFlashProgOK;


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
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 LOADING BLU DATA...", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
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
    snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 UPGRADING BLU FIRMWARE", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
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
	if (var == 0)
		snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 UPGRADING BLU SUCCESS...", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
	else
		snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 UPGRADING BLU FAIL, RETRY", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2);
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

static void blu_get_verion()
{
    unsigned char version[21] = "\0";
    unsigned char addr[1] = {0x80};
    int ret;
    int i = 0;

    ret = MDrv_IIC_ReadBytesPort0(BLU_FW_SLAVEADDR, 1, addr, 0x14, version);
    if (!ret) {
        UBOOT_ERROR("BLU: get BLU version fail\n");
        return;
    }

    for (i = 0; i < 20; i++)
        printf("%c", (char)version[i]);
    printf("\n");
}

static void blu_update_init(unsigned int size)
{
    if ((size % 0x80) == 0)
        blankNumber = size / 0x80;
    else
        blankNumber = size / 0x80 + 1;

    BluUpgradeState = BluFlashProgStateInit;
    BluErrorFlag = BluFlashProgOK;
    printf("BLU: blu_update_init OK!\n");
    mdelay(200);
}

static int start_fw_upgrade(void)
{
    int ret = 0;
    unsigned char data[3] = {0x01, 0x01, 0x90};
    unsigned char addr[1] = {0x00};

    addr[0] = FW_UPDATE;
    ret = MDrv_IIC_WriteBytesPort0(BLU_FW_SLAVEADDR, 1, addr, 3, data);
    if (!ret) {
        UBOOT_ERROR("BLU: start_fw_upgrade write fail\n");
        return ret;
    }

    printf("BLU: start_fw_upgrade OK\n");
    mdelay(200);
    return ret;
}

static int blu_back_read(void)
{
    unsigned char addr[1] = {0x00};
    unsigned char value[1] = {0x00};
    int ret;

    MDrv_HWI2C_SetReadMode(E_HWI2C_READ_MODE_DIRECT);   // Set read mode as direct read
    mdelay(30);
    ret = MDrv_IIC_ReadBytesPort0(BLU_SLAVEADDR, 1, addr, 1, value);
    printf("BLU1: ret = %d, value=%d\n", ret, value[0]);
    mdelay(20);
    MDrv_HWI2C_SetReadMode(E_HWI2C_READ_MODE_DIRECTION_CHANGE); // set read mode back to default
    if (value[0])
        return value[0];

    return value[0];
}

static int blu_flash_erase()
{
    int ret = 0;
    unsigned char data[3] = {0x01, 0x01, 0x06};
    unsigned char addr[1] = {0x00};

    addr[0] = ERASE_APP;
    ret = MDrv_IIC_WriteBytesPort0(BLU_SLAVEADDR, 1, addr, 3, data);
    if (!ret) {
        UBOOT_ERROR("BLU: blu_flash_erase ERASE_APP write fail\n");
    }

    mdelay(3000);
    printf("BLU: Flash Chip Erase OK, need wait 3s for call read value\n");
    //back read value
    printf("start blu_flash_erase back read value\n");
    ret = blu_back_read();
    return ret;
}

static int blu_programing(int num)
{
    unsigned char checkSum = WRITE_PM;
    int ret;
    unsigned char count = 3;
    unsigned char addr[1] = {0x00};
    unsigned char pcCounter = 0;
    int i = 0;
    unsigned char tdata[COMMANDNUM + 1];

    checkSum ^= num;
    tdata[pcCounter] = num;
    for (pcCounter = 1; pcCounter < COMMANDNUM; pcCounter++) {
        tdata[pcCounter] = BluFileBuf[num * 0x80 + pcCounter - 1];
        checkSum ^= tdata[pcCounter];
    }
    tdata[pcCounter] = checkSum;
    printf("checkSum=0x%x\n",checkSum);
    for (i = 0; i < count; i++) {
        printf("BLU: blu_programing count=%d\n", i);
        addr[0] = WRITE_PM;
        ret = MDrv_IIC_WriteBytesPort0(BLU_SLAVEADDR, 1, addr, sizeof(tdata), tdata);
        if (!ret)
            UBOOT_ERROR("BLU: write WRITE_PM fail\n");
        mdelay(100);
        printf("start blu_programing back read value\n");
        ret = blu_back_read();
        if (ret == 1) {
            mdelay(25);
            break;
        }
    }
    return ret;
}

static int blu_flash_program(void)
{
    int ret = 0;
    int num = 0;

    for (num = 0; num < blankNumber; num++) {
		show_Upgrading(num, blankNumber);
        ret = blu_programing(num);
        if (!ret)
            break;
    }
    if (ret)
        printf("blu_flash_program OK.....");

	show_Upgrading(num, blankNumber);
    return ret;
}

static void blu_boot_retry()
{
    unsigned char data[3] = {0x01, 0x01, 0x07};
    unsigned char value[1] = {0x00};
    unsigned char addr[1] = {0x00};
    int ret;

    addr[0] = BOOT_RETRY;
    ret = MDrv_IIC_WriteBytesPort0(BLU_SLAVEADDR, 1, addr, 3, data);
    if (!ret)
        UBOOT_ERROR("BLU: blu_boot_retry write fail\n");

    mdelay(1000);

    printf("start blu_boot_retry back read value\n");
    ret = blu_back_read();
}

static void blu_reset()
{
    unsigned char data[3] = {0x01, 0x01, 0x04};
    unsigned char value[1] = {0x00};
    unsigned char addr[1] = {0x00};
    int ret;

    addr[0] = GO_RESET;
    ret = MDrv_IIC_WriteBytesPort0(BLU_SLAVEADDR, 1, addr, 3, data);
    if (!ret)
        UBOOT_ERROR("BLU: blu_reset write fail\n");
    mdelay(30);
    printf("start blu_reset back read value\n");
    ret = blu_back_read();
}

static int blu_isp_program()
{
    static errorCounter = 0;
    int ret = 0;

    printf("BLU: BluErrorFlag=%d, BluFlashProgOK=%d, state=%d\n", BluErrorFlag, BluFlashProgOK, BluUpgradeState);
    if (BluErrorFlag != BluFlashProgOK) {
        UBOOT_ERROR("BLU: errorflag is not OK");
        return (0xF0 + BluFlashProgOK);
    }

    switch (BluUpgradeState) {
        case BluFlashProgStateInit:
        {
            printf("\nBLU_1: FlashProgStateInit fw upgrade\n");
            currentPercent = 100;
            start_fw_upgrade();
            mdelay(2000);
            BluUpgradeState = BluFlashProgStateErase;
            break;
        }
        case BluFlashProgStateErase:
        {
            printf("\nBLU_2: FlashProgStateErase fw upgrade\n");
            mdelay(1500);
            ret = blu_flash_erase();
            mdelay(1000);
            if (ret) {
                currentPercent = 80;
                BluUpgradeState = BluFlashProgStateProgram;
            } else
                BluErrorFlag = BluFlashEraseErr;

            break;
        }
        case BluFlashProgStateProgram:
        {
			show_StartUpgrading(0);
            printf("\nBLU_3: program **********************\n");
            ret = blu_flash_program();
            if (ret) {
                BluUpgradeState = BluFlashProgStateReset;
                currentPercent = 70;
            } else {
                errorCounter++;
                if (errorCounter < 6) {
                    blu_boot_retry();
                    mdelay(20);
                    BluUpgradeState = BluFlashProgStateErase;
                    currentPercent = 80;
                } else {
                    BluErrorFlag = BluFlashBlankingErr;
                    BluUpgradeState = BluFlashProgStateIdle;
                    goto program_end;
                }
            }
            break;
        }
        case BluFlashProgStateReset:
        {
            printf("\nBLU_4: FlashProgStateReset***********\n");
            mdelay(200);
            blu_reset();
            mdelay(2000);
            currentPercent = 0;
            BluUpgradeState = BluFlashProgStateIdle;
            blu_get_verion();
            break;
        }
        default:
            break;
    }

program_end:
    if (BluErrorFlag != BluFlashProgOK)
        return (0xF0 + BluErrorFlag);
    else
        return currentPercent;
}

static int blu_upgrade_func(unsigned char *fp, unsigned int size)
{
    int i = 0;
    int ret;
    int watchdog_disable_status = 0;

    if (!fp || !size) {
        UBOOT_ERROR("BLU: file pointer or size is error\n");
        return BLU_NOT_OK;
    }

    if (size > MAX_UPGRADE_IMAGE_FILE_SIZE) {
        UBOOT_ERROR("BLU: binary file size overflow, size=0x%x > 0x%x\n",
                   size, MAX_UPGRADE_IMAGE_FILE_SIZE);
        return BLU_NOT_OK;
    }

    if ((size % 0x80) == 0) {
        BluFileBuf = (unsigned char *)malloc(size); 
        if (BluFileBuf == NULL) {
            UBOOT_ERROR("BLU: malloc fail size=0x%x\n", size);
            return BLU_NOT_OK;
        }
        memset(BluFileBuf, 0, size);
    } else {
        BluFileBuf = (unsigned char *)malloc(size + 0x80 - (size % 0x80));
        if (BluFileBuf == NULL) {
            UBOOT_ERROR("BLU: malloc fail size=0x%x\n", size);
            return BLU_NOT_OK;
        }
        memset(BluFileBuf, 0, size + 0x80 - (size % 0x80));
    }

    INV_CTL_On();
    mdelay(200);
    if (MDrv_WDT_IsEnable()) {
        run_command("wdt_enable 0", 0);
        watchdog_disable_status = 1;
        printf("BLU: disable watchdog\n");
    }

    printf("BLU: size=0x%x\n", size);
    for (i = 0; i < size; i++)
        BluFileBuf[i] = fp[i];

    blu_update_init(size);
	show_LoadData(0);

    do {
        ret = blu_isp_program();
    } while ((ret > 0) && (ret < 0xF0));

    if (ret == 0)
        printf("BLU: *********** BL Updating Success ***********");
    else
        printf("BLU: *********** BL Updating Fail ret=0x%x***********\n", ret);

	show_Finish(ret);
    mdelay(3000);
    if (watchdog_disable_status == 1) {
        printf("BLU: enable watchdog\n");
        run_command("wdt_enable", 0);
    }

    return ret;
}

int do_blu_usb_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cmd[CMD_BUF] = "\0";
    char *env =NULL;
    int partition = 0;
    int device = 0;
    int ret = 0;
    MS_U8 u8Percent= 0;
    long unsigned  int u32FileSize = 0;
    unsigned char *fp = NULL;
    char ConfigName[BUFFER_SIZE] = "\0";

    if(!idme_get_var_external("config_name", ConfigName, (BUFFER_SIZE - 1))) {
        if (strncmp(ConfigName, "haileyplus_", 11) &&
            strncmp(ConfigName, "hailey_ABC", 14)) {
            UBOOT_ERROR("BLU: don't support blu upgrade feature.\n");
            return -1;
        }
    }
    printf("BLU: only Haileyplus/ABC/ABC will support upgrade BLU FW: %s\n",
          ConfigName);

    if (-1 == appInitUsbDisk()) {
        UBOOT_ERROR("FAIL : can not init usb!! \n");
        return -1;
    }

    ret = check_file_partition(BLU_BIN_PATH, &device, &partition);
    if(ret == 0) {
            UBOOT_DEBUG("Geting file size\n");
            snprintf(cmd, CMD_BUF, "fatfilesize usb %d:%d %s", device, partition, BLU_BIN_PATH);
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

            snprintf(cmd, CMD_BUF, "fatload usb %d:%d  %X %s %ul",device, partition, UPGRADE_BUFFER_ADDR, BLU_BIN_PATH,u32FileSize);

            UBOOT_DEBUG("cmd=%s\n",cmd);
            if(-1 == run_command(cmd, 0)) {
                UBOOT_ERROR("Load Upgrade File fail!\n");
                return -1;
            } else {
                printf("Start Upgrade backlight driver IC firmware!~\n");
                run_command("panel_pre_init", 0);//panel power on
                mdelay(1000);
                /* HWIIC init. */
                MDrv_IIC_InitPort0(E_HWI2C_PORT0_3);
                printf("BLU: Before update BLU, version is: ");
				mdelay(100);
                blu_get_verion();
                fp = (unsigned char *)UPGRADE_BUFFER_ADDR;
                if (blu_upgrade_func(fp, u32FileSize) != 0) {
                    UBOOT_ERROR("BLU: upgrade fail.............please retry\n");
                    return -1;
                } else {
                    printf("BLU: update OK...............\n");
                    return 0;
                }
            }
    } else {
        UBOOT_DEBUG("no %s in usb disk\n",BLU_BIN_PATH);
        return -1;
    }
}
