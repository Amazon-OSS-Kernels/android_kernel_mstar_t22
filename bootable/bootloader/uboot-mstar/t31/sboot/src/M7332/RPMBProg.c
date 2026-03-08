const unsigned char rpmbprog[]__attribute__((section("rpmbprog_dat"))) =
{
    #include "teeloader_rpmbprog.dat"
};
const unsigned char rpmbprog_SIG[]__attribute__((section("rpmbprog_dat"))) =
{
    #include "teeloader_rpmbprog_sig.dat"
};