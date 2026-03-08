const unsigned char TEE_loader[]__attribute__((section("tee_loader_dat"))) =
{
    #include "tee_loader.dat"
};
const unsigned char TEE_loader_padding[0x300]__attribute__((section("tee_loader_dat"))) =
{
    0x0
};
const unsigned char TEE_loader_SIG[]__attribute__((section("tee_loader_dat"))) =
{
    #include "tee_loader_sig.dat"
};

