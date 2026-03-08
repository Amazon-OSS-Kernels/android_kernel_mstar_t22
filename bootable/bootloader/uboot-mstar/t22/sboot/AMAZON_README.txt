README
======
THESE INSTRUCTIONS AND RELATED BUILD SCRIPTS ARE PROVIDED BY AMAZON ON AN
"AS IS" BASIS. AMAZON MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND,
EXPRESS OR IMPLIED, AS TO THESE INSTRUCTIONS, RELATED BUILD SCRIPTS, OR ANY
THIRD PARTY TECHNOLOGY SUCH AS ANDROID OPEN SOURCE PROJECT CODE OR THIRD PARTY
COMPILERS REFERENCED THEREIN (COLLECTIVELY, “BUILD MATERIALS”). YOU EXPRESSLY
AGREE THAT YOUR USE OF THE BUILD MATERIALS IS AT YOUR SOLE RISK.

AMAZON WILL NOT BE LIABLE FOR ANY DAMAGES OF ANY KIND ARISING FROM THE USE OF
THE BUILD MATERIALS INCLUDING, BUT NOT LIMITED TO, DIRECT, INDIRECT,
INCIDENTAL, PUNITIVE, AND CONSEQUENTIAL DAMAGES.

BUILDING UBoot
-------------------

1.  Obtain a copy of gcc arm_eabi-2011.03 compiler or a substitute cross-compiler.
        Recommended download link: https://github.com/AdiPat/Android_Toolchains/tree/master/arm-2011.03
        Add its bin path to environment variable PATH

	export PATH="<path/to/arm-eabi-2011.03/bin>:$PATH"

2.  Update mk_config and paste in the path to the root of the copy of the compiler in the
    CROSS_COMPILER_PATH variable.

3.  Ensure that sh is symlinked to bash in the /bin directory.

    a. If it is not symlinked, run:

       ln -s /bin/bash /bin/sh

    b. If it is already symlinked to something other than bash, run:

       sudo ln -sf /bin/bash /bin/sh

3.  Execute the make script for <Public Name> by running:

    ./mk sophia

4.  Output can be found in out/unsigned/sboot.bin.unsigned and out/unsigned/u-boot.bin
