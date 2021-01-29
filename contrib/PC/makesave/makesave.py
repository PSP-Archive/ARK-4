#!/usr/bin/env python2

import sys, struct, os, shutil, gzip, StringIO
from discread import *

CMA_PSP_SAVE_ROOT = os.getenv("CMA_PSP_SAVE_ROOT", "")

DATA_BIN=\
"""
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
"""

PARAM_SFO = \
"""
H4sIAMcXEUkC/2MICHZjZGRgmMLAwMABpDkYQICFiRlEMkAAJwMLCwuUD8ISQHkBBggHpF4DyOcD
0g4gM4Bi1kCJBB4GMPYA8v2A/AagHAivAGqMA6rnhfI1BBkYcoF8KZg8kO/sGOLq7h8UyRDgGOTq
F+LoE+/jGubqwxDsGObq4hjiGO/iGuLoicz3DHJ1DgHpgAu5efq4xvt4BocghICmOfoGI/ghniE+
rgwQkgEowcAKtD/c3wcIuQwUAvw9/UKCGUbBKBgFo4BYACpZ9Jw8/cAc52mhshPD3FVOsrlva92Z
LzAaPKNgFIyCUTAKRsEoGAWjYBSMAhKAIxq/Yd2mTev5Nu31iTvskvesRH+N4N+Wv0+zla41brzq
LvX7LaX28fIf6UnyeXA4O0F0cmrr3gp3R19XhYAgf9D4ykgMfwAJ2HCEMBMAAA==
"""

def usage():
	print ("Usage: %s <infile> [short_name] [detail_name]" % (sys.argv[0]))
	print ("\t - Transfer ISO to CMA psp savedata directory")

def replaceBinary(data, pos, replace):
	return data[0:pos] + replace + data[pos + len(replace):]

SHORT_NAME_OFFSET = 0x510
SHORT_NAME_MAX_LENGTH = 0x40
LONG_NAME_OFFSET = 0x12B0
LONG_NAME_MAX_LENGTH = 0x80

def gzipDecompress(data):
	sio = StringIO.StringIO(data)

	with gzip.GzipFile(fileobj=sio, mode='rb') as gz:
		data = gz.read()
		return data

def getISOFile(isofn, path):
	iso = ISO(isofn)
	re = iso.findPath(path)

	if re:
		lba, fileSize = re
		return iso.read(lba * ISO_SECTOR_SIZE, fileSize)
	else:
		return None

def getPARAM_SFO(isofn):
	return getISOFile(isofn, "/PSP_GAME/PARAM.SFO")

def getICON0(isofn):
	return getISOFile(isofn, "/PSP_GAME/ICON0.PNG")

def getParamData(data, pos, size):
	return data[pos:pos+size]

def getString(data, pos):
	r = []

	while pos < len(data) and data[pos] != '\0':
		r.append(data[pos])
		pos += 1

	return "".join(r)

def getGameTitle(param):
	s = struct.Struct("<IIIII")
	s = s.unpack(param[0:s.size])
	nameOff, valOff, count = s[2:5]

	s = struct.Struct("<HBBIIHH")
	i = 0

	while i < count:
		pos = 0x14+i*s.size
		v = s.unpack(param[pos:pos+s.size])
		name = getString(param, nameOff + v[0])

		if name == "TITLE":
			return getParamData(param, valOff + v[5], v[3])
		i += 1

	return None

def createSave(isofn, shortfn, longfn):
	sfo = gzipDecompress(PARAM_SFO.decode("base64"))
	data_bin=DATA_BIN.decode("base64")

	if not shortfn:
		shortfn = os.path.splitext(os.path.split(isofn)[-1])[0]

	shortname = shortfn[0:min(SHORT_NAME_MAX_LENGTH, len(shortfn))]

	if not longfn:
		try:
			param = getPARAM_SFO(isofn)
		except RuntimeError as e:
			param = None

		if not param:
			longfn = os.path.splitext(os.path.split(isofn)[-1])[0]
			longfn += "\00"
		else:
			longfn = getGameTitle(param)

	longname = longfn[0:min(LONG_NAME_MAX_LENGTH, len(longfn))]

	sfo = replaceBinary(sfo, SHORT_NAME_OFFSET, shortname + "\00")
	sfo = replaceBinary(sfo, LONG_NAME_OFFSET, longname)
	sfo = replaceBinary(sfo, 0x48, struct.pack('<B', len(shortname)+1))
	sfo = replaceBinary(sfo, 0x88, struct.pack('<B', len(longname)))
	shortname = os.path.join(CMA_PSP_SAVE_ROOT, shortname)

	try:
		os.mkdir(shortname)
	except OSError as e:
		pass

	with open(os.path.join(shortname, "PARAM.SFO"), "wb") as of:
		of.write(sfo)

	with open(os.path.join(shortname, "DATA.BIN"), "wb") as of:
		of.write(data_bin)

	try:
		icon0 = getICON0(isofn)
	except RuntimeError as e:
		icon0 = None

	if icon0:
		with open(os.path.join(shortname, "ICON0.PNG"), "wb") as of:
			of.write(icon0)

	shutil.copy(isofn, shortname)
	print ("%s transfer as %s done" %(isofn, shortname))

def checkString(string):
	for s in string:
		if s not in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' and s not in '0123456789' and s not in '.':
			return False
	return True

'''Check filename for 1.80 CMA limit
1. 8.3 filename
2. ALL IN UPPER
3. only A-Z, 0-9 allowed
'''
def checkFilename(fn):
	basename = os.path.basename(fn)

	if not checkString(basename):
		return False

	prefix, suffix = os.path.splitext(basename)

	if len(suffix) > 3 + 1:
		return False

	if len(prefix) > 8:
		return False

	return True 

def main():
	if len(sys.argv) < 2:
		usage()
		sys.exit(1)

	isofn = sys.argv[1]

	if not checkFilename(isofn):
		print ("WARNING: CMA for FW 1.80 only accepts 8.3 file name with only UPPERCASE letters and digits")

	if len(sys.argv) >= 3:
		shortfn = sys.argv[2]
	else:
		shortfn = ""

	if len(sys.argv) >= 4:
		longfn = sys.argv[3]
	else:
		longfn = ""

	createSave(isofn, shortfn, longfn)

if __name__ == "__main__":
	main()
