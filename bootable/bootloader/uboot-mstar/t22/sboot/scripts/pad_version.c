typedef unsigned char   U8;
typedef unsigned int   U16;
typedef unsigned long  U32;
#include <stdio.h>
#include <string.h>
#include "../src/version.h"

void printUsage(char *progName)
{
    printf("Usage:\n\t%s [path to sboot.bin] [MBOOT_CL]\n\n", progName);
}

void printInvalidCL(char *mbootCL)
{
    printf("%s is not a valid MBOOT_CL\n\n", mbootCL);
}

int main(int argc, char **argv)
{
    U8 i, *ptr;
    FILE *fp;
    char *changelist = NULL , *changelistTmp=NULL;

    changelist = argv[2];
    if(argc !=3 || NULL == changelist)
    {
        printUsage(argv[0]);
        return 1;
    }
    fp = fopen(argv[1], "r+w");
    //printf("MBOOT_CL = %s\n",argv[2]);
    //MBOOT_CL sample: MBOT-1106.0.10.1250503 or MBOT-1106.0.10.:1252082$
    for(i=0 ; i < 3 ; i++ )
    {
        changelist = strstr(changelist , ".");
        if(NULL == changelist)
        {
            printInvalidCL(argv[2]);
            break;
        }
        changelist++;
    }
    //for $Change:$ case
    changelistTmp = strstr(changelist , ":");
    if(NULL != changelistTmp)//omit :
    {
        changelist = changelistTmp + 1;
    }
    changelistTmp = strstr(changelist , "$");
    if(NULL != changelistTmp)//omit $
    {
        *changelistTmp = 0x00;
    }
    //
    if(strlen(changelist)>sizeof(_sbt_version_msif.BOOT.changelist))//trim changelist if needed
    {
        changelist[sizeof(_sbt_version_msif.BOOT.changelist)]=0x00;
    }
    //printf("changelist = %s\n",changelist);
    ptr = _sbt_version_msif.BOOT.changelist + sizeof(_sbt_version_msif.BOOT.changelist) - strlen(changelist);
    strncpy(ptr , changelist , strlen(changelist));
    if(fp != NULL)
    {
        ptr = (U8*)&_sbt_version_msif.BOOT;	
        fseek(fp, 0x10000-sizeof(_sbt_version_msif.BOOT), SEEK_SET); // (end of sboot.bin) - (length of version string)
        for(i=0; i<sizeof(_sbt_version_msif.BOOT); i++)
        {
            if(ptr[i] != (U8)fputc((int)ptr[i], fp))
                printf("Err(%d)\n", i);
        }

        fclose(fp);
    }
    else
    {
        printf("fclose() failed on file '%s'\n", argv[1]);
    }
    return 0;
}
