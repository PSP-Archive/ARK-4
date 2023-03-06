#!/usr/bin/env python3

import argparse
import functools
import struct

parser = argparse.ArgumentParser(description='TimeMachine IPL Payload Creator')

parser.add_argument('--input_payload', help='Payload binary', type=str, required=True)
parser.add_argument('--payload_addr', help='Destination address of the payload', type=functools.partial(int, base=16), required=True)
parser.add_argument('--ipl_addr', help='Destination address of the Sony IPL', type=functools.partial(int, base=16), default=0)
parser.add_argument('--output', help='Output file', type=str, required=True)

args = parser.parse_args()

def payload_header(payload_addr, ipl_addr):
    return struct.pack('<II', payload_addr, ipl_addr)

with open(args.input_payload, 'rb') as f:
    payload_code = f.read()

header = payload_header(args.payload_addr, args.ipl_addr)

with open(args.output, 'wb') as f:
    f.write(header + payload_code)
