#!/usr/bin/env python3

import argparse

from psptool.pbp import is_pbp, PBP
from psptool.pack import pack_prx
from psptool.prx import encrypt

parser = argparse.ArgumentParser(description="Infinity Updater Packer")
parser.add_argument('input', type=argparse.FileType('rb'),
                    help='The raw PBP to pack')
parser.add_argument('--vanity', type=str,
                    help='Some vanity text in the executable header')
parser.add_argument('output', type=str,
                    help='The output to write the packed PBP')
args = parser.parse_args()

executable = args.input.read()

if not is_pbp(executable):
    raise ValueError("not a PBP")

pbp = PBP(executable)
pbp.prx = encrypt(pack_prx(pbp.prx, is_pbp=True,
                           psptag=lambda x: 0x0B000000), vanity=args.vanity)

with open(args.output, 'wb') as f:
    f.write(pbp.pack())
