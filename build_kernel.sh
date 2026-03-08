#!/bin/bash
################################################################################
#
#  build_kernel.sh
#
#  Copyright (c) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

################################################################################
#
#  I N P U T
#
################################################################################
# Folder for platform tarball.
PLATFORM_TARBALL="${1}"

# Target directory for output artifacts.
TARGET_DIR="${2}/kernel"

################################################################################
#
#  V A R I A B L E S
#
################################################################################

# Retrieve the directory where the script is currently held
SCRIPT_BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Configuration file for the build.
CONFIG_FILE="${SCRIPT_BASE_DIR}/build_kernel_config.sh"
PATCH_FILE="${SCRIPT_BASE_DIR}/platform_patch.txt"

# Workspace directory & relevant temp folders.
WORKSPACE_DIR="$(mktemp -d)"
PLATFORM_EXTRACT_DIR="${WORKSPACE_DIR}/src"
mkdir -p "${PLATFORM_EXTRACT_DIR}"

# Remove workspace directory upon completion.
trap "rm -rf $WORKSPACE_DIR" EXIT

PARALLEL_EXECUTION="-j8"

function usage {
    echo "Usage: ${BASH_SOURCE[0]} path_to_platform_tar output_folder" 1>&2
    exit 1
}

function validate_input_params {
    if [[ ! -f "${PLATFORM_TARBALL}" ]]
    then
        echo "ERROR: Platform tarball not found."
        usage
    fi

    if [[ ! -f "${CONFIG_FILE}" ]]
    then
        echo "ERROR: Could not find config file ${CONFIG_FILE}. Please check" \
             "that you have extracted the build script properly and try again."
        usage
    fi
}

function display_config {
    echo "-------------------------------------------------------------------------"
    echo "SOURCE TARBALL: ${PLATFORM_TARBALL}"
    echo "TARGET DIRECTORY: ${TARGET_DIR}"
    echo "KERNEL SUBPATH: ${KERNEL_SUBPATH}"
    echo "DEFINITION CONFIG: ${DEFCONFIG_NAME}"
    echo "TARGET ARCHITECTURE: ${TARGET_ARCH}"
    echo "TOOLCHAIN REPO: ${TOOLCHAIN_REPO}"
    echo "TOOLCHAIN PREFIX: ${TOOLCHAIN_PREFIX}"
    echo "-------------------------------------------------------------------------"
    echo "Sleeping 3 seconds before continuing."
    sleep 3
}

function setup_output_dir {
    if [[ -d "${TARGET_DIR}" ]]
    then
        FILECOUNT=$(find "${TARGET_DIR}" -type f | wc -l)
        if [[ ${FILECOUNT} -gt 0 ]]
        then
            echo "ERROR: Destination folder is not empty. Refusing to build" \
                 "to a non-clean target"
            exit 3
        fi
    else
        echo "Making target directory ${TARGET_DIR}"
        mkdir -p "${TARGET_DIR}"

        if [[ $? -ne 0 ]]
        then
            echo "ERROR: Could not make target directory ${TARGET_DIR}"
            exit 1
        fi
    fi
}

function download_toolchain {
    echo "Cloning toolchain ${TOOLCHAIN_REPO} to ${TOOLCHAIN_DIR}"
    git clone "${TOOLCHAIN_REPO}" "${TOOLCHAIN_DIR}" -b pie-release
    if [[ $? -ne 0 ]]
    then
        echo "ERROR: Could not clone toolchain from ${TOOLCHAIN_REPO}."
        exit 2
    fi
}

function extract_tarball {
    echo "Extracting tarball to ${PLATFORM_EXTRACT_DIR}"
    tar xf "${PLATFORM_TARBALL}" -C ${PLATFORM_EXTRACT_DIR}
}

function apply_patch {
    if [[ -f "${PATCH_FILE}" ]]
    then
        echo "Applying patch to ${PLATFORM_EXTRACT_DIR}"
        pushd ${PLATFORM_EXTRACT_DIR}
        patch -p1 < ${PATCH_FILE}
        popd
    fi
}

function exec_build_kernel {
    CCOMPILE="${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}"
    MAKE_ARGS="ARCH=${TARGET_ARCH} CROSS_COMPILE=${CCOMPILE}"

    # Move into the kernel sub folder.
    pushd "${PLATFORM_EXTRACT_DIR}/${KERNEL_SUBPATH}"

    # Step 1: Run genlink_kernel.sh
    sh ./genlink_kernel.sh

    # Step 2: defconfig
    cp -f "arch/${TARGET_ARCH}/configs/${DEFCONFIG_NAME}" .config

    # Step 3: run olddefconfig
    echo "Make oldconfig: make ${MAKE_ARGS} olddefconfig"
    make ${MAKE_ARGS} olddefconfig

    # Step 3A: output config, for reference
    echo ".config contents"
    echo "---------------------------------------------------------------------"
    cat .config
    echo "---------------------------------------------------------------------"

    # Step 4: full make
    echo "Running full make"
    make ${MAKE_ARGS} ${PARALLEL_EXECUTION}

    if [[ $? != 0 ]]; then
        echo "ERROR: Failed to build kernel" >&2
        exit 1
    fi

    popd
}

function copy_to_output {
    echo "Copying Kernel files to output"
    pushd "${PLATFORM_EXTRACT_DIR}/${KERNEL_SUBPATH}"
    find "./arch/"${TARGET_ARCH}"/boot" -type f | sed 's/^\.\///' | while read CPFILE
    do
        local BASEDIR="$(dirname "${CPFILE}")"
        if [[ ! -d "${TARGET_DIR}/${BASEDIR}" ]]
        then
            mkdir -p "${TARGET_DIR}/${BASEDIR}"
        fi
        cp -v "${CPFILE}" "${TARGET_DIR}/${CPFILE}"
    done
    popd
}

function validate_output {
    echo "Listing output files"
    local IFS=":"
    for IMAGE in ${KERNEL_IMAGES};do
        if [ ! -f ${TARGET_DIR}/${IMAGE} ]; then
            echo "ERROR: Missing kernel output image ${IMAGE}" >&2
            exit 1
        fi
        ls -l ${TARGET_DIR}/${IMAGE}
    done
}

################################################################################
#
#  M A I N
#
################################################################################

# Phase 1: Set up execution
validate_input_params
source "${CONFIG_FILE}"
setup_output_dir
TARGET_DIR="$(cd "${TARGET_DIR}" && pwd)"
display_config

# Phase 2: Set up environment
if [ -n "${TOOLCHAIN_NAME}" ]; then
    TOOLCHAIN_DIR="/tmp/${TOOLCHAIN_NAME}"
fi
if [ -z "$(ls -A ${TOOLCHAIN_DIR})" ]; then
    download_toolchain
fi
extract_tarball
apply_patch

# Phase 3: build kernel
exec_build_kernel

# Phase 4: move to output
copy_to_output

# Phase 5: verify output
validate_output
