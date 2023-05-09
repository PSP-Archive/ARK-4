#!/usr/bin/env python3

import argparse

from psptool.pack import pack_prx
from psptool.prx import decrypt

parser = argparse.ArgumentParser(description="Infinity Kernel Module Packer")
parser.add_argument('input', type=argparse.FileType('rb'),
                    help='The raw kernel PRX to pack')
parser.add_argument('output', type=str,
                    help='The output to write the packed PRX')
args = parser.parse_args()

executable = args.input.read()
executable = decrypt(executable)

with open(args.output, 'wb') as f:
    f.write(executable)
