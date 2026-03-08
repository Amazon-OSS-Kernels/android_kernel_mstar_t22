#include <linux/printk.h>
#include <linux/version.h>
#include <linux/module.h>
#include <generated/autoconf.h>
#include "MsTypes.h"
#include "mdrv_iic_io.h"

#ifdef CONFIG_MP_PNL_STR_CUST

#define HIBYTE(value)  ((MS_U8)((value) / 0x100))
#define LOBYTE(value)  ((MS_U8)(value))


/*
  1.What is pnl_cust:

  KApi_PNL_Cust_xxx is used for adding customization code at panel str flow.

  we name it by the related position.

  for exampe: KApi_PNL_Cust_Resume_Setting_VCC_onTiming1
  It means the customization function is active between VCC and onTiming1.

  2.others notes:

      2.1 need define CONFIG_MP_PNL_STR_CUST at config (only can define at customer branch now)
      2.2 the caller is at utopia Ex:UTPA2-700.0.x\modules\xc\api\pnl\apiPNL.c
*/


/*
  Normal Panel Suspend flow:

  -----------------------------
  Backlight
  -----------------------------
  offTiming1 delay
  -----------------------------
  Data
  -----------------------------
  offTiming2 delay
  -----------------------------
  VCC
  -----------------------------

  if all customization insert at str suspend like bellow:

  ------
  KApi_PNL_Cust_Suspend_Setting_Before_Backlight
  ------
  -----------------------------
  Backlight
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_Backlight_OffTiming1
  ------
  -----------------------------
  offTiming1 delay
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_OffTiming1_Data
  ------
  -----------------------------
  Data
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_Data_OffTiming2
  ------
  -----------------------------
  offTiming2 delay
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_OffTiming2_VCC
  ------
  -----------------------------
  VCC
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_After_VCC
  ------

*/

MS_BOOL KApi_PNL_Cust_Suspend_Setting_Before_Backlight(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_Backlight_OffTiming1(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_OffTiming1_Data(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_Data_OffTiming2(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_OffTiming2_VCC(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_After_VCC(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}


/*

  Normal Panel Resume flow:

  -----------------------------
  VCC
  -----------------------------
  onTiming1 delay
  -----------------------------
  Data
  -----------------------------
  onTiming2 delay
  -----------------------------
  Backlight
  -----------------------------

  if all customization insert at str resume like bellow:

  ------
  KApi_PNL_Cust_Suspend_Setting_Before_VCC
  ------
  -----------------------------
  VCC
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_VCC_onTiming1
  ------
  -----------------------------
  onTiming1 delay
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_OnTiming1_Data
  ------
  -----------------------------
  Data
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_Data_OnTiming2
  ------
  -----------------------------
  onTiming2 delay
  -----------------------------
  ------
  KApi_PNL_Cust_Suspend_Setting_OnTiming2_Backlight
  ------
  -----------------------------
  Backlight
  -----------------------------
  ------
  KApi_PNL_Cust_Resume_Setting_After_Backlight
  ------
*/
MS_BOOL KApi_PNL_Cust_Resume_Setting_Before_VCC(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_VCC_OnTiming1(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_OnTiming1_Data(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_Data_OnTiming2(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_OnTiming2_Backlight(void)
{
    MS_BOOL bRet = FALSE;
    return bRet;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_After_Backlight(void)
{
    MS_BOOL bRet = FALSE;

//Here is a example for using IIC cmd to externel device
/*
   MS_U16 u16BusNumSlaveID = 0x266;
   MS_U8 u8BusNum = HIBYTE(u16BusNumSlaveID);
   MS_U8 u8SlaveID = LOBYTE(u16BusNumSlaveID);

   MS_U8 u8Offset=0x00;
   MS_U8 u8Reg_Value[42]={0x48,0x19,0xA4,0x00,0x00,0x23,0xF8,0x66,0x7B,0x2B,
                          0x28,0x00,0x10,0x10,0x07,0x07,0x3E,0xD3,0xD5,0x35,
                          0x13,0x06,0x2D,0xB2,0x73,0x24,0x81,0xEA,0x1A,0xE1,
                          0x4C,0x11,0xC0,0xC7,0x02,0xB0,0x0D,0x1C,0x51,0xB2,
                          0x4D,0x1D};
 
    pr_info("*****CONFIG_MP_PNL_STR_CUST define*****\n");
    int ret = MDrv_SW_IIC_WriteBytes(u8BusNum, u8SlaveID, 1, &u8Offset, sizeof(u8Reg_Value), u8Reg_Value);
    if (ret >= 0)
    {
        bRet = TRUE;
    }
    else
    {
        pr_err("*****\SWI2C error*****\n",__FILE__, __func__);
        bRet = FALSE;
    }
*/
    return bRet;
}
#else
MS_BOOL KApi_PNL_Cust_Suspend_Setting_Before_Backlight(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_Backlight_OffTiming1(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_OffTiming1_Data(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_Data_OffTiming2(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_OffTiming2_VCC(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Suspend_Setting_After_VCC(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_Before_VCC(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_VCC_OnTiming1(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_OnTiming1_Data(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_Data_OnTiming2(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_OnTiming2_Backlight(void)
{
    return FALSE;
}

MS_BOOL KApi_PNL_Cust_Resume_Setting_After_Backlight(void)
{
    pr_info("*****CONFIG_MP_PNL_STR_CUST not define*****\n");
    return FALSE;
}
#endif
EXPORT_SYMBOL(KApi_PNL_Cust_Suspend_Setting_Before_Backlight);
EXPORT_SYMBOL(KApi_PNL_Cust_Suspend_Setting_Backlight_OffTiming1);
EXPORT_SYMBOL(KApi_PNL_Cust_Suspend_Setting_OffTiming1_Data);
EXPORT_SYMBOL(KApi_PNL_Cust_Suspend_Setting_Data_OffTiming2);
EXPORT_SYMBOL(KApi_PNL_Cust_Suspend_Setting_OffTiming2_VCC);
EXPORT_SYMBOL(KApi_PNL_Cust_Suspend_Setting_After_VCC);


EXPORT_SYMBOL(KApi_PNL_Cust_Resume_Setting_Before_VCC);
EXPORT_SYMBOL(KApi_PNL_Cust_Resume_Setting_VCC_OnTiming1);
EXPORT_SYMBOL(KApi_PNL_Cust_Resume_Setting_OnTiming1_Data);
EXPORT_SYMBOL(KApi_PNL_Cust_Resume_Setting_Data_OnTiming2);
EXPORT_SYMBOL(KApi_PNL_Cust_Resume_Setting_OnTiming2_Backlight);
EXPORT_SYMBOL(KApi_PNL_Cust_Resume_Setting_After_Backlight);



