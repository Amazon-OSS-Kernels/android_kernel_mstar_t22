#if defined(CONFIG_DECRYPT_BY_TLAESKEY)
unsigned char AES_TLENC[] __attribute__((aligned(16)))= {
    #include "../../AES_TLENC.dat"
};
#endif