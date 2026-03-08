#!/usr/bin/python

import sys
import os
import re
from os.path import join
from subprocess import Popen, PIPE
import array

PREFIX            = '  [TEE_LOADER]'
CONFIG_PATH       = '.config'
SBOOT_ELF         = 'sboot.elf'
SBOOT_BIN         = 'sboot.bin'
TEE_LOADER_OUT    = 'tee_loader.dat'
TEE_LOADER_OFFSET = 0x10000

def get_nmap(NM, path):
    out = Popen([NM, path], stdout=PIPE).communicate()[0]
    return dict([(m[2], (m[0], m[1]))
        for m in re.findall('([0-9a-zA-Z]{8})\s*(.)\s*(.+)', out)])

def main(argv):

    bindir = argv[1]


    sboot_nmap = get_nmap(os.environ['NM'], join(bindir, SBOOT_ELF))
    _ld_LDR_load_start = int('0x' + sboot_nmap['_ld_TEE_LDR_load_start'][0], 16)
    _ld_LDR_load_end = int('0x' + sboot_nmap['_ld_TEE_LDR_load_end'][0], 16)
    teeloader_size = _ld_LDR_load_end - _ld_LDR_load_start;

    print PREFIX, 'args:'
    print '    bindir = %s' % bindir
    print '    _ld_LDR_load_start = 0x%08x' % _ld_LDR_load_start
    print '    _ld_LDR_load_end = 0x%08x' % _ld_LDR_load_end
    print '    teeloader_size = 0x%08x' % teeloader_size

    flash = open(join(bindir, SBOOT_BIN), 'rb').read()
    tee_loader_bin = flash[(TEE_LOADER_OFFSET):(TEE_LOADER_OFFSET + teeloader_size)]
    with open(join(bindir,TEE_LOADER_OUT), 'w') as output:
        cnt = 0
        for c in tee_loader_bin:
            output.write("0x{:02x}, ".format(ord(c)))
            cnt +=1
            if cnt == 16:
                output.write('\n')
                cnt = 0

if __name__ == '__main__':
    main(sys.argv)
