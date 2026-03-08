BOOT_OUTPUT_PATH=./out

Num_Tbl=2   #This value will be auto-update by gen_TBL.sh. Try not to modify it.
Num_Type_512=1
Tbl_Size_512=0x200
Num_Type_1024=10
Tbl_Size_1024=0x400
Sign_Size=0x0100

TOOLDIR=./util
SECUREDIR=./secure
RSA_BOOT_PRIVATE_KEY=$SECUREDIR/RSAboot_priv.txt
ALLTBL_BIN=$BOOT_OUTPUT_PATH/AllTbl.bin
TMP_BIN=$BOOT_OUTPUT_PATH/tmp.bin

function show_info()
{
     echo ============================================
     printf "offset= %x\n" $offset
     printf "data size to be sign = %x\n" $sign_off
     echo   "sign name: " ${TMP_BIN}.sign.bin
     printf "Sign_size= %x\n" $Sign_Size
     printf "sign will be append at: %x\n" $sign_addr
}

function sign_tbl_512()
{

  for ((i=0; i<$(($Num_Type_512*$Num_Tbl)); i++));
  do
     sign_addr=$(($offset+$sign_off_512))

     show_info

     if [ -f $TMP_BIN ]; then
         rm $TMP_BIN
     fi

     $TOOLDIR/SeparateBin.exe $ALLTBL_BIN $TMP_BIN $offset $sign_off_512
     $SECUREDIR/rsa_sign $TMP_BIN $RSA_BOOT_PRIVATE_KEY
     scripts/append_signature.pl ${TMP_BIN}.sig.bin $Sign_Size $ALLTBL_BIN $(printf "%x" $sign_addr)
     offset=$(($offset+$Tbl_Size_512))
  done
}

function sign_tbl_1024()
{

  for ((i=0; i<$(($Num_Type_1024*$Num_Tbl)); i++));
  do
     sign_addr=$(($offset+$sign_off_1024))

     show_info

     if [ -f $TMP_BIN ]; then
         rm $TMP_BIN
     fi

     $TOOLDIR/SeparateBin.exe $ALLTBL_BIN $TMP_BIN $offset $sign_off_1024
     $SECUREDIR/rsa_sign $TMP_BIN $RSA_BOOT_PRIVATE_KEY
     scripts/append_signature.pl ${TMP_BIN}.sig.bin $Sign_Size $ALLTBL_BIN $(printf "%x" $sign_addr)
     offset=$(($offset+$Tbl_Size_1024))
  done
}

offset=0
sign_off_512=$(($Tbl_Size_512-0x100))
sign_off_1024=$(($Tbl_Size_1024-0x100))

sign_tbl_512
sign_tbl_1024
