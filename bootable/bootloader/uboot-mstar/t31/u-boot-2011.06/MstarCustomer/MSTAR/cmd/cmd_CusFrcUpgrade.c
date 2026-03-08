#include <command.h>
#include <CusFrcUpgrade.h>

U_BOOT_CMD(
    frc_upgrade ,    CONFIG_SYS_MAXARGS,    0,     do_frc_upgrade,
    "Upgrade external frc firmware in tvconfig partition.",
    ""
);
