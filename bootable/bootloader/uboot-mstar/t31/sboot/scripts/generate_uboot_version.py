import sys
import struct

uboot_version = 0

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        uboot_version = int(sys.argv[1], 0)

    with open('uboot_version.bin', 'wb') as fp:
        fp.write(struct.pack('I', uboot_version))
