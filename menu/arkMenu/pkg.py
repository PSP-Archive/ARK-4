#!/usr/bin/env python2

import os

IMAGE_DIRECTORY = "resources/"
DEST_FILE = "DATA.PKG"

def int2bin(number):
	hex = '%08X' %(number)
	return chr(int('0x' + hex[6:8], 16)) + chr(int('0x' + hex[4:6], 16)) + chr(int('0x' + hex[2:4], 16)) + chr(int('0x' + hex[0:2], 16))

dir = os.listdir(IMAGE_DIRECTORY)
dir.sort()

track = []

pkg = open(DEST_FILE, 'wb')
pkg.close()

pkg = open(DEST_FILE, 'w+')

for i in xrange(len(dir)):
	if os.path.isfile(IMAGE_DIRECTORY+dir[i]) and dir[i] != DEST_FILE:
		track.append(pkg.tell())
		pkg.write(int2bin(0) + int2bin(len(dir[i])))
		pkg.write(dir[i]+"\0")

pkg.write(int2bin(0xFFFFFFFF))

track.reverse()

for i in xrange(len(dir)):
	if os.path.isfile(IMAGE_DIRECTORY+dir[i]) and dir[i] != DEST_FILE:
		fd = open(IMAGE_DIRECTORY+dir[i], 'rb')
		cur = pkg.tell()
		pkg.seek(track.pop())
		pkg.write(int2bin(cur))
		pkg.seek(cur)
		pkg.write(fd.read())
		fd.close()

pkg.close()
