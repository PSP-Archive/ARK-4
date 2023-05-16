#!/usr/bin/env python3

import argparse
import struct
from psptool.kirk import kirk1_encrypt_cmac, kirk4
from Crypto.Hash import SHA1

parser = argparse.ArgumentParser(description='IPL packer')

parser.add_argument('input', nargs='+', help='Input binary', type=str)
parser.add_argument('output', help='Output file', type=str)
parser.add_argument('jump_address', help='Address to jump to', type=lambda x : int(x, 0))

args = parser.parse_args()

def ipl_header(address, size, entry, sum):
	return struct.pack('<IIII', address, size, entry, sum)

def createBlock(address, data, last_sum):
	header = ipl_header(address, len(data), 0, last_sum) 
	block = header + data
	key = bytearray.fromhex('AA'*16)
	block_enc = kirk1_encrypt_cmac(block, aes_key=key, cmac_key=key)

	sha_buf = block[8:8+8+len(data)] + block[:8]

	block_hash = SHA1.new(sha_buf).digest()
	hash_padding = bytearray(12)
	hash_enc = kirk4(block_hash + hash_padding, 0x6C)

	# pad with zeros to IPL block size minus the SHA1 hash at the end
	padding = bytearray(0x1000 - len(block_enc) - 0x20)
	
	data_sum = 0
	for i in range(int(len(data) / 4)):
		data_sum += int.from_bytes(data[i*4:(i+1)*4], byteorder='little')
	
	return bytearray(block_enc + padding + hash_enc), data_sum & 0xFFFFFFFF

def createResetExploit():
	# reset exploit; 0x32F6 is USB_HOST, ATA_HDD, MS_1, ATA, USB, AVC, VME, SC, ME reset bit
	header = ipl_header(0xBC10004C, 4, 0x10000005, 0) + struct.pack('<IIII', 0x32F6, 0, 0, 0) 
	block = header + b'\x38\x00\xf0\x0b\x00\x00\x00\x00\x00\x80\x08\x40\x00\x08\x09\x24\x80\x11\x08\x7d\x04\x48\x09\x01\x21\x40\x00\x00\x00\x00\x10\xbd\x00\xe0\x0a\x40\x00\xe8\x0b\x40\x00\x05\x4c\x7d\x04\x00\x80\x11\x00\x05\x6c\x7d\x40\x53\x0a\x00\x25\x50\x48\x01\x00\x00\x5a\xbd\x03\x00\x80\x11\x40\x5b\x0b\x00\x25\x58\x68\x01\x00\x00\x7a\xbd\x40\x00\x08\x25\xf1\xff\x09\x15\x00\x00\x00\x00\x08\x00\xe0\x03\x0f\x00\x00\x00\x00\x80\x08\x40\x00\x10\x09\x24\x40\x12\x08\x7d\x04\x48\x09\x01\x00\xe0\x80\x40\x00\xe8\x80\x40\x21\x40\x00\x00\x00\x00\x01\xbd\x00\x00\x03\xbd\x40\x00\x08\x25\xfc\xff\x09\x15\x00\x00\x00\x00\x08\x00\xe0\x03\x00\x00\x00\x00\x10\xbc\x02\x3c\x40\x00\x44\x8c\x00\xff\x03\x3c\x24\x18\x83\x00\x02\x00\x60\x10\x10\x00\x02\x3c\x02\x12\x04\x00\x08\x00\xe0\x03\x00\x00\x00\x00\xf8\xff\xbd\x27\x04\x00\xbf\xaf\x2f\x00\xf0\x0f\x00\x00\x00\x00\x60\x00\x03\x3c\x2b\x10\x43\x00\x05\x3c\x03\x3c\x17\x00\x40\x14\xc0\xbf\x64\x34\x01\x80\x02\x3c\xc0\xbf\x63\x24\x88\x00\x40\xac\xac\x00\x44\xac\x00\x01\x43\xac\x34\x01\x43\xac\x10\x3c\x03\x3c\xc0\xbf\x63\x34\x58\x01\x43\xac\x00\x08\x03\x3c\x71\x40\x63\x24\x94\x01\x43\xac\x01\x00\x03\x24\x8c\x0a\x43\xac\x0a\x00\xf0\x0f\x00\x00\x00\x00\x21\x00\xf0\x0f\x00\x00\x00\x00\x04\x00\xbf\x8f\x01\x80\x19\x3c\x08\x00\x20\x03\x08\x00\xbd\x27\x01\x80\x02\x3c\xc0\xbf\x63\x24\x88\x00\x40\xac\xac\x00\x44\xac\xb8\x00\x43\xac\x10\x3c\x03\x3c\xc0\xbf\x63\x34\xcc\x00\x43\xac\x01\x00\x03\x24\x0c\x08\x43\xac\xed\xff\x00\x10\x00\x00\x00\x00'
	block += bytearray.fromhex('00'*(0x100 - len(block)))
	key = bytearray.fromhex('AA'*16)
	block_enc = kirk1_encrypt_cmac(block, aes_key=key, cmac_key=key)

	sha_buf = block[8:8+8+4] + block[:8]

	block_hash = SHA1.new(sha_buf).digest()
	hash_padding = bytearray(12)
	hash_enc = kirk4(block_hash + hash_padding, 0x6C)

	# pad with zeros to IPL block size minus the SHA1 hash at the end
	padding = bytearray(0x1000 - len(block_enc) - 0x20)

	return bytearray(block_enc + padding + hash_enc)

def createJump(address, last_sum):
	header = ipl_header(0, 0, address, last_sum) 
	block = header + bytearray.fromhex('00'*0x100)
	key = bytearray.fromhex('AA'*16)
	block_enc = kirk1_encrypt_cmac(block, aes_key=key, cmac_key=key)

	sha_buf = block[8:8+8] + block[:8]

	block_hash = SHA1.new(sha_buf).digest()
	hash_padding = bytearray(12)
	hash_enc = kirk4(block_hash + hash_padding, 0x6C)

	# pad with zeros to IPL block size minus the SHA1 hash at the end
	padding = bytearray(0x1000 - len(block_enc) - 0x20)
	
	return bytearray(block_enc + padding + hash_enc)

with open(args.output, 'wb') as of:
	of.write(createResetExploit())
	last_sum = 0 
	for input in args.input:
		file, addr = input.split('@')
		addr = int(addr, 16)
		with open(file, 'rb') as rf:
			while True:
				block = rf.read(0xE00)
				if not block:
					break;
				block, last_sum = createBlock(addr, block, last_sum)
				of.write(block)
				addr += 0xE00
	of.write(createJump(args.jump_address, last_sum))

