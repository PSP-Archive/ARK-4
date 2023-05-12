#!/usr/bin/env python3

import argparse, struct

from psptool.pack import pack_prx
from psptool.prx import encrypt

PSP_HEADER_MAGIC = 0x5053507E

parser = argparse.ArgumentParser(description="Infinity Kernel Module Packer")
parser.add_argument('input', type=argparse.FileType('rb'),
                    help='The raw kernel PRX to pack')
parser.add_argument('output', type=str,
                    help='The output to write the packed PRX')
args = parser.parse_args()

class PSPHeader:
    def __init__(self, data):
        self.attribute = 0x1000
        self.modinfo_offset = 0
        self.version = 1

        # set compression to gzip
        self.comp_attribute = 0
        self.module_ver_lo = 0 & 0xFF
        self.module_ver_hi = (0 >> 8) & 0xFF
        self.modname = ''
        self.elf_size = len(data)

        self.entry = 0
        self.nsegments = 0
        self.seg_align = [0] * 4
        self.seg_address = [0] * 4
        self.seg_size = [0] * 4

        self.bss_size = 0

        self.devkitversion = 0
        self.decrypt_mode = 2

        self.psptag = 0x4C9494F0
        self.oetag = 0xCCCCCCCC
        self.comp_size = 0
    
        padding = (0x10-(self.elf_size % 16)) if self.elf_size % 16 else 0
        self.psp_size = self.elf_size + padding + 0x150

        self.reserved = [0]*5
        self.key_data0 = [0xDA]*0x30
        self.reserved2 = [0]*2
        self.key_data1 = [0xDA]*0x10
        self.signcheck = [0]*0x58
        self.key_data2 = 0
        self.key_data3 = [0xDA]*0x1C

    def pack(self):
        return struct.pack('<IHHBB28sBBIIIII4H4I4I5IIBxH48BII2I16BI88BII28B',
                           PSP_HEADER_MAGIC, self.attribute, self.comp_attribute, self.module_ver_lo, self.module_ver_hi,
                           self.modname.encode('ascii'), self.version, self.nsegments, self.elf_size, self.psp_size, self.entry,
                           self.modinfo_offset, self.bss_size, *self.seg_align, *self.seg_address, *self.seg_size,
                           *self.reserved, self.devkitversion, self.decrypt_mode, 0,
                           *self.key_data0, self.comp_size, 0x80, *self.reserved2, *self.key_data1, self.psptag,
                           *self.signcheck, self.key_data2, self.oetag, *self.key_data3)

id = bytearray.fromhex('AA'*16)

data = args.input.read()

psp_header = PSPHeader(data)

data = encrypt(psp_header.pack() + data, id=id)

with open(args.output, 'wb') as f:
    f.write(data)
