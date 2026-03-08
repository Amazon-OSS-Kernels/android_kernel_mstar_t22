#!/bin/bash

################################################################################
#
#  mk
#
#  Copyright (c) 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
################################################################################

# Configuration file for the build.
CONFIG_FILE="${PWD}/mk_config"
source "${CONFIG_FILE}"

TOOLSCHAINPATH="${CROSS_COMPILER_PATH}/bin/"
echo "[S]TOOLSCHAINPATH = ${TOOLSCHAINPATH}"
export PATH="${TOOLSCHAINPATH}:${PATH}"

cp ${KERNEL_CONFIG} .config
sh ${GENLINK_KERNEL}

echo "[S]olddefconfig start"
make olddefconfig
echo "[S]make clean start"
make clean
echo "[S]make start"
make -j1

