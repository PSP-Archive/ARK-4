#!/usr/bin/env python3

import argparse

from psptool.pack import pack_prx
from psptool.prx import encrypt

parser = argparse.ArgumentParser(description="Infinity Kernel Module Packer")
parser.add_argument('input', type=argparse.FileType('rb'),
                    help='The raw kernel PRX to pack')
parser.add_argument('--tag', type=str,
                    help='tag in the executable header')
parser.add_argument('output', type=str,
                    help='The output to write the packed PRX')
args = parser.parse_args()

id = bytearray.fromhex('AA'*16)

tag = 0x00000000
if args.tag:
	tag = int(args.tag, 16)

executable = args.input.read()
executable = encrypt(pack_prx(executable, is_pbp=False, fix_relocs=False, psptag=lambda x: tag), id=id)

with open(args.output, 'wb') as f:
    f.write(executable)
