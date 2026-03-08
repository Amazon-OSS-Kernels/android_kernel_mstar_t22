# **********************************************
# Environment
# **********************************************
target_defConfig=scripts/defconfig
function do_Recovery()
{
	if [  -f autoconf.h_backup ]; then
		echo "Recovery autoconf.h"
		mv autoconf.h_backup include/autoconf.h
	fi

	if [  -f ".config_backup" ]; then
		echo "Recovery .config"
		mv .config_backup .config
	fi

	if [  -f $target_defConfig"_backup" ]; then
		echo "Recovery $target_defConfig"
		#backup scripts/defconfig
		mv  $target_defConfig"_backup" $target_defConfig
	fi
}
do_Recovery