#!/usr/bin/env python3

import argparse
import struct
from psptool.kirk import kirk1

parser = argparse.ArgumentParser(description='IPL packer')

parser.add_argument('input', help='Input binary', type=str)
parser.add_argument('output', help='Output file', type=str)
parser.add_argument('--xor', help='xor key', type=lambda x : int(x, 0))

args = parser.parse_args()

def bitrev32(n):
	return int('{:032b}'.format(n)[::-1], 2)

def __ROR4__(value, count):
	return ((value & (2**32-1)) >> count%32) | (value << (32-(count%32)) & (2**32-1))

g_XorSeeds = [
	0x61A0C918, 0x45695E82, 0x9CAFD36E, 0xFA499B0F,
	0x7E84B6E2, 0x91324D29, 0xB3522009, 0xA8BC0FAF,
	0x48C3C1C5, 0xE4C2A9DC, 0x00012ED1, 0x57D9327C,
	0xAFB8E4EF, 0x72489A15, 0xC6208D85, 0x06021249,
	0x41BE16DB, 0x2BD98F2F, 0xD194BEEB, 0xD1A6E669,
	0xC0AC336B, 0x88FF3544, 0x5E018640, 0x34318761,
	0x5974E1D2, 0x1E55581B, 0x6F28379E, 0xA90E2587,
	0x091CB883, 0xBDC2088A, 0x7E76219C, 0x9C4BEE1B,
	0xDD322601, 0xBB477339, 0x6678CF47, 0xF3C1209B,
	0x5A96E435, 0x908896FA, 0x5B2D962A, 0x7FEC378C,
	0xE3A3B3AE, 0x8B902D93, 0xD0DF32EF, 0x6484D261,
	0x0A84A153, 0x7EB16575, 0xB10E53DD, 0x1B222753,
	0x58DD63D0, 0x8E8B8D48, 0x755B32C2, 0xA63DFFF7,
	0x97CABF7C, 0x33BDC660, 0x64522286, 0x403F3698,
	0x3406C651, 0x9F4B8FB9, 0xE284F475, 0xB9189A13,
	0x12C6F917, 0x5DE6B7ED, 0xDB674F88, 0x06DDB96E,
	0x2B2165A6, 0x0F920D3F, 0x732B3475, 0x1908D613
]

def getXorKey(Spare0):
	start_idx = (Spare0 >> 5) & 0x3F
	ror_cnt = Spare0 & 0x1F
	keys = g_XorSeeds[start_idx:start_idx+4]

	xor_key = [
		__ROR4__(keys[0], ror_cnt),
		bitrev32(__ROR4__(keys[1], ror_cnt)),
		__ROR4__(keys[2], ror_cnt) ^ keys[3],
		__ROR4__(keys[3], ror_cnt)
	]

	return bytearray(b''.join([x.to_bytes(4, 'little') for x in xor_key]))

with open(args.input, 'rb') as rf:
	with open(args.output, 'wb') as of:
		while True:
			block = bytearray(rf.read(0x1000))
			if not block:
				break;

			if args.xor:
				key = getXorKey(args.xor)

				for i in range(16):
					block[i] ^= key[i]
					
			block[0x62] = 0
					
			block = kirk1(block)
			
			address, size, entry, sum = struct.unpack('<IIII', block[:16])
			data = block[16:16+size]
			
			of.write(data)
