#ifndef _BOOT_INC_
#include "boot.inc"
#endif

#if defined (CONFIG_MSTAR_BUILD_TEE_LOADER_DAT)
//================================================================================================
//_vector
//================================================================================================
.globl      _vector
_vector:

    b  teeloader_entry
#endif

