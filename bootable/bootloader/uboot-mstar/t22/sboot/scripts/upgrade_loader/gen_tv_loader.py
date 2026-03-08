import os
from sys import argv, exit
from os.path import join, getsize, dirname, abspath
from shutil import copyfile

def help():
    print """\
Usage:
  %s chip_name upgrade_mode  O_path
  -chip_name            Chip Name, like M7221 (reversed, not used for now)
  -upgrade_mode         Generate PC or Disk Mode of upgrade loader
  -hash0_path           Path of hash0 (related_path)
  -secure_tool_path     Path of secure tools (related_path)
  -O_path               Path of output file (related_path)

Example:
  %s M7221 disk ../../secure/ ../../out/
    """ % (argv[0], argv[0])
    exit(0)

def usage():
    print str(argv[0]) + ': missing operand'
    print 'Try using arguments below for more information:'
    print ' -h   usage & example'
    exit(1)


def change_hash2_size_in_hash1(fileName, output_path):
    with open(fileName, "rb") as binary_file:
        hash1_bin = binary_file.read(0xDF00)

    list_hash1_bin = list(hash1_bin)
    hash2_name = join(output_path,"mboot.bin")
    hash2_size = getsize(hash2_name)
    hash2_size = '{0:08x}'.format(int(hash2_size))
    print 'hash2_size =' , hash2_size[0:2], hash2_size[2:4], hash2_size[4:6], hash2_size[6:8]
    # we need to modify 4 byte in sboot.bin at 0xddfc~ddff
    list_hash1_bin[0xDDFC:0xDE00] = hash2_size.decode('hex')[::-1]

    hash1_bin = ''.join(list_hash1_bin)
    with open(fileName, "wb") as binary_file:
        binary_file.write(hash1_bin)

def resign_hash1(fileName, secure_tool_path):
    RSA_tool = "%s/rsa_sign" % secure_tool_path
    RSA_Key = "%s/RSAboot_priv.txt" % secure_tool_path
    run_cmd = "./%s %s %s" % (RSA_tool, fileName, RSA_Key)
    #print run_cmd
    os.system(run_cmd)

def main(argv):

    if len(argv) < 2:
        usage()
    elif len(argv) == 2:
        if argv[1] == '-h':
            help()

    if len(argv) != 6:
        print 'Wrong number of arguments!'
        exit(1)

    CHIP_NAME           = argv[1]
    UPGRADE_MODE        = argv[2]
    HASH0_PATH          = argv[3]
    RELATED_SECURE_PATH = argv[4]
    RELATED_OUTPUT_PATH = argv[-1]

    now_dir = os.path.dirname(__file__)
    secure_tool_path = os.path.join(now_dir, RELATED_SECURE_PATH)
    output_path = os.path.join(now_dir, RELATED_OUTPUT_PATH)

    hash1_name = join(output_path,"hash1.bin")
    hash1_sig_name = join(output_path,"hash1.bin.sig.bin")
    change_hash2_size_in_hash1(hash1_name, output_path)

    resign_hash1(hash1_name, secure_tool_path)

    if UPGRADE_MODE == "pc":
        hash0_ext = ".inb"
        hash1_ext = ".inb"
        hash2_ext = ".tlb"
    elif UPGRADE_MODE == "disk":
        hash0_ext = ".bin"
        hash1_ext = ".bin"
        hash2_ext = ".bin"
    else:
        print "Error upgrade mode!!!"
        exit(2)

    #HASH0_PATH = "../../secure/hash/%s/SZ/" % CHIP_NAME
    REAL_HASH0_PATH = os.path.join(now_dir, HASH0_PATH)

    ud_hash0_name = "ud_hash0" + hash0_ext
    ud_hash1_name = "ud_hash1" + hash1_ext
    ud_hash2_name = "ud_hash2" + hash2_ext

    #copy hash0 into ud_hash0
    copyfile(REAL_HASH0_PATH, join(output_path, ud_hash0_name))

    # we need to cat new hash1.bin and new hash1.bin.sig.bin to ud_hash1.bin
    with open(hash1_name, "rb") as binary_file:
        hash1_bin = binary_file.read()

    with open(hash1_sig_name, "rb") as binary_file:
        hash1_sig_bin = binary_file.read()

    padding_bin = 0x100 * '\x00'
    hash1_bin += padding_bin
    hash1_bin += hash1_sig_bin

    with open(join(output_path, ud_hash1_name), "wb") as binary_file:
        binary_file.write(hash1_bin)

    copyfile(join(output_path,"mboot.bin"), join(output_path,ud_hash2_name))

    exit(0)

if __name__ == "__main__":
    main(argv)
