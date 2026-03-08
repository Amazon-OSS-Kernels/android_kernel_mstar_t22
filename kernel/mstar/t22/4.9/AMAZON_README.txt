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

BUILDING Kernel
-------------------

1.  Obtain a copy of gcc linaro-aarch64_linux-2014.09 compiler or a substitute cross-compiler.
        Recommended download link: https://releases.linaro.org/archive/14.09/components/toolchain/binaries/
        Add its bin path to environment variable PATH

	export PATH="<path/to/linaro-aarch64_linux-2014.09/bin>:$PATH"

2.  Update mk_config and paste in the path to the root of the copy of the compiler in the
    CROSS_COMPILER_PATH variable.

3.  Ensure that sh is symlinked to bash in the /bin directory.

    a. If it is not symlinked, run:

       ln -s /bin/bash /bin/sh

    b. If it is already symlinked to something other than bash, run:

       sudo ln -sf /bin/bash /bin/sh

4.  Execute the make script for <Public Name> by running:

    ./mk

5.  Output can be found in arch/arm64/boot/Image.gz
