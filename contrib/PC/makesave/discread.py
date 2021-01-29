#!/usr/bin/env python2

from hexutil import toHex, toBinary
from struct import unpack, pack, Struct
from zlib import decompress
import os, sys

ISO = 1
CISO = 2

ISO_SECTOR_SIZE = 0x800
ISO_MAGIC_OFFSET = 0x8000
ISO_MAGIC = toBinary("0143443030310100")
CISO_MAGIC = "CISO"

ISO9660_FILEFLAGS_DIR = 2

class ISORecord:
	def __init__(__self__, struct):
		__self__.lsbStart = struct[2]
		__self__.lsbDataLength = struct[4]
		__self__.len_fi = struct[18]
		__self__.fileFlags = struct[13]
		__self__.len_dr = struct[0]

class CISOHeader:
	def __init__(__self__, struct):
		__self__.totalSector = struct[5] / struct[6]
		__self__.align = struct[8]

def normalizeName(name):
	r = []

	for ch in name:
		if ch == '\0':
			break
		r.append(ch)

	return "".join(r)

def zipDecompress(compressed):
	return decompress(compressed, -15)

class ISO:
	def __init__(__self__, fn):
		fp = open(fn, "rb")
		__self__.fp = fp
		__self__.isoType = __self__.GetISOType()

		if __self__.isoType == ISO:
			fp.seek(0, os.SEEK_END)
			__self__.totalSector = fp.tell() / ISO_SECTOR_SIZE
			pass
		elif __self__.isoType == CISO:
			s = Struct("<BBBBIQIBBBB")
			data = __self__.readRaw(0, s.size)
			__self__.cisoHeader = CISOHeader(s.unpack(data))
			__self__.cacheIdx = []
			__self__.totalSector = __self__.cisoHeader.totalSector

			for i in xrange(__self__.totalSector+1):
				off = unpack('<I', __self__.readRaw(s.size + i * 4, 4))[0]
				__self__.cacheIdx.append(off)
		else:
			raise RuntimeError("Unknown file")

		data = __self__.readSector(16)
		s = Struct("<BBIIIIBBBBBBBBBBHHB")
		__self__.rootRecord = ISORecord(s.unpack(data[0x9C:0x9C+s.size]))
		assert(__self__.rootRecord.lsbDataLength == ISO_SECTOR_SIZE)

	def getTotalSector(__self__):
		if __self__.isoType == ISO:
			return __self__.totalSector
		elif __self__.isoType == CISO:
			return __self__.cisoHeader.totalSector

	def readSector(__self__, lba):
		return __self__.read(lba * ISO_SECTOR_SIZE, ISO_SECTOR_SIZE)
	def readCSOSector(__self__, lba):
		if __self__.cacheIdx[lba] & 0x80000000:
			# uncompressed
			off = (__self__.cacheIdx[lba] & ~0x80000000) << __self__.cisoHeader.align
			return __self__.readRaw(off, ISO_SECTOR_SIZE)

		off = (__self__.cacheIdx[lba] & ~0x80000000) << __self__.cisoHeader.align
		nextOff = (__self__.cacheIdx[lba+1] & ~0x80000000) << __self__.cisoHeader.align
		size = nextOff - off

		if __self__.cisoHeader.align:
			size += 1 << __self__.cisoHeader.align

		size = max(ISO_SECTOR_SIZE, size)
		data = __self__.readRaw(off, size)

		return zipDecompress(data)

	def read(__self__, pos, size):
		if __self__.isoType == CISO:
			lba = pos / ISO_SECTOR_SIZE

			if size % ISO_SECTOR_SIZE == 0:
				counter = size / ISO_SECTOR_SIZE
			else:
				counter = size / ISO_SECTOR_SIZE + 1

			i = 0
			data = []

			while i < counter:
				data.append(__self__.readCSOSector(lba + i))
				i += 1

			data = "".join(data)
			data = data[0:size]

			return data

			raise RuntimeError("CISO")
		else:
			return __self__.readRaw(pos, size)
	def readRaw(__self__, pos, size):
		fp = __self__.fp
		fp.seek(pos)
		return fp.read(size)
	def GetISOType(__self__):
		magic = __self__.readRaw(0x8000, 8)

		if magic == ISO_MAGIC:
			return ISO

		magic = __self__.readRaw(0, 4)

		if magic == CISO_MAGIC:
			return CISO

		raise RuntimeError("Unknown file type")

	def findFile(__self__, path, lba, dirSize, isDir):
		data = __self__.read(lba * ISO_SECTOR_SIZE, dirSize)
		pos = 0

		while pos < dirSize:
			s = Struct("=BBIIIIBBBBBBBBBBHHB")
			record = ISORecord(s.unpack(data[pos:pos+s.size]))

			if record.len_dr == 0:
				pos = (pos / ISO_SECTOR_SIZE + 1) * ISO_SECTOR_SIZE
				continue

			name = data[pos+s.size:pos+s.size+record.len_fi]
			name = normalizeName(name)

			if path == name:
				if isDir:
					if not (record.fileFlags & ISO9660_FILEFLAGS_DIR):
						return None
				else:
					if (record.fileFlags & ISO9660_FILEFLAGS_DIR):
						return None

				return record
			pos += record.len_dr
	
	def parsePath(__self__, path):
		paths = [ el for el in path.split('/') if el ]

		paths2pass = []

		for p in paths:
			if p == ".." and len(paths2pass) > 0:
				del(paths2pass[-1])
			elif p == ".":
				continue
			else:
				paths2pass.append(p)

		return paths2pass
	
	def findPath(__self__, path):
		paths = __self__.parsePath(path)
		lba = __self__.rootRecord.lsbStart
		dirSize = __self__.rootRecord.lsbDataLength

		for i in range(len(paths)):
			path = paths[i]
			rec = __self__.findFile(path, lba, dirSize, 1 if i != len(paths)-1 else 0)

			if rec:
				lba = rec.lsbStart
				dirSize = rec.lsbDataLength
			else:
				break

		if rec:
			return rec.lsbStart, rec.lsbDataLength
		else:
			return None

def main():
	iso = ISO(sys.argv[1])
	r = iso.findPath(sys.argv[2])

	if r:
		lba, fileSize = r
		eboot = iso.read(lba * ISO_SECTOR_SIZE, fileSize)
		print ("%d bytes dumped" % (fileSize))

		with open(os.path.basename(sys.argv[2]), "wb") as f:
			f.write(eboot)
	else:
		print ("Not found")
if __name__ == "__main__":
	main()
