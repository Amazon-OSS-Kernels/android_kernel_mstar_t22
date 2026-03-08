#if defined(CONFIG_AUTH_BY_TLRSAKEY)
unsigned char RSA_TLPUB[] __attribute__((aligned(16)))= {
    #include "../../RSA_TLPUB.dat"
};
#endif