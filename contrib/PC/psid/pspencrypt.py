#!/usr/bin/python
import hashlib, sys, random, os, struct

PSID_SALT_MAGIC = "\x5D\xB1\x1D\xC0"
ENCRYPTED_TAG_MAGIC = "\x5D\xB1\x1D\xC0"

def toBinary(data):
	l = []
	i = 0

	while i < len(data):
		l.append(chr(int(data[i:i+2], 16)))
		i += 2

	return "".join(l)

def toHex(data):
	l = []
	i = 0

	while i < len(data):
		l.append("%02X" % (ord(data[i])))
		i += 1

	return "".join(l)

def parsePSID(psid):
	if len(psid) == 2 * 16:
		raise RuntimeError("Invalid PSID not 16 bytes length")

	i = 0
	l = []

	while i < len(psid):
		l.append(chr(int(psid[i:i+2], 16)))
		i += 2

	return "".join(l)

def rc4crypt(data, key):
    x = 0
    box = range(256)
    for i in range(256):
        x = (x + box[i] + ord(key[i % len(key)])) % 256
        box[i], box[x] = box[x], box[i]
    x = 0
    y = 0
    out = []
    for char in data:
        x = (x + 1) % 256
        y = (y + box[x]) % 256
        box[x], box[y] = box[y], box[x]
        out.append(chr(ord(char) ^ box[(box[x] + box[y]) % 256]))
    
    return ''.join(out)

def psidHash(psid):
	return hashlib.sha1(PSID_SALT_MAGIC + psid).digest()[0:16]

def xorBytes(a, b):
	assert(len(a) == len(b))
	l = []
	i = 0

	while i < len(a):
		t = (ord(a[i]) ^ ord(b[i])) & 0xFF
		l.append("%c" %(t))
		i += 1

	return "".join(l)

def isPlainPRX(f):
	f.seek(0)
	if f.read(4) != "\x7E\x50\x53\x50":
		return False
	f.seek(0x150)
	if f.read(2) != "\x1f\x8b":
		return False
	f.seek(0xD0)
	if f.read(4) != "\xF0\xDA\xDA\xDA":
		return False

	return True

def encrypt(fn, outfn, psidstr, isPrx = True):
	psid = toBinary(psidstr)
	r = os.urandom(16)
	h = psidHash(psid)
	fkey = xorBytes(r, h)

	'''
	for c in r:
		print "%02X" % (ord(c)),
	print("")
	for c in h:
		print "%02X" % (ord(c)),
	print("")
	for c in fkey:
		print "%02X" % (ord(c)),
	'''
	
	assert(len(fkey) == 16)

	with open(fn, "rb") as f:
		if isPrx:
			if not isPlainPRX(f):
				print ("not a plain prx")
				sys.exit(1)
			
			f.seek(0)
			hdr = f.read(0x150)
			f.seek(0xb0)
			compsize = struct.unpack('I', f.read(4))[0]

			f.seek(0x150)
		else:
			compsize = os.path.getsize(fn)
			

		d = f.read(compsize)
		e = rc4crypt(d, r)

	with open(outfn, "wb") as fo:
		if isPrx:
			fo.write(hdr)
			fo.write(e)
			fo.seek(0xD0)
			fo.write(ENCRYPTED_TAG_MAGIC)
			fo.seek(0x80)
			fo.write(fkey)
			fo.seek(0x130)
			fo.write('\xD1\x51\xB9\x56\x3F\x04\x7D\x4D\x31\xB7\x20\xAD\xB3\xC0\x9E\x65\xCC\x7F\xC6\x48\x5B\xCF\xB2\x9C\xCE\x78\x0E\x39\xB8\xFA\x3F\xE6') # 4 bytes is magic for IsPrxEncrypted
		
		else:
			fo.write(e)
			fo.write(fkey)



def isEncryptedPrx(f):
	f.seek(0)
	if f.read(4) != "\x7E\x50\x53\x50":
		return False
	f.seek(0xD0)
	if f.read(4) != "\x5D\xB1\x1D\xC0":
		return False

	return True

def decrypt(fn, outfn, psidstr):
	psid = toBinary(psidstr)
	with open(fn, "rb") as f:
		if not isEncryptedPrx(f):
			print ("not an encrypted prx")
			sys.exit(1)

		f.seek(0)
		hdr = f.read(0x150)

		f.seek(0x80)
		fkey = f.read(16)

		f.seek(0x150)
		e = f.read()

		h = psidHash(psid)
		key = xorBytes(h, fkey)
		d = rc4crypt(e, key)

	with open(outfn, "wb") as fo:
		fo.write(hdr)
		fo.write(d)
		fo.seek(0xD0)
		fo.write("\xF0\xDA\xDA\xDA")
		fo.seek(0x130)
		fo.write("\x5D\xB1\x1D\xC0\x57\xD7\xA2\xE1\x7C\x3E\x14\x15\x90\x48\x09\x5D\x1E\x90\xDF\x27\x01\xB7\x52\xCB\xD2\x32\x6B\x2F\xDA\x36\xA2\x64")

def usage():
	print ("%s: -e|-d prx outfile psid" % (sys.argv[0]))
	print ("\t-e: encrypt")
	print ("\t-d: decrypt")
	print ("\t-eb: encrypt binary")

def main():
	if len(sys.argv) < 5:
		usage()
		sys.exit(1)

	if sys.argv[1] == '-e':
		encrypt(sys.argv[2], sys.argv[3], sys.argv[4])
	elif sys.argv[1] == '-d':
		decrypt(sys.argv[2], sys.argv[3], sys.argv[4])
	elif sys.argv[1] == '-eb':
		encrypt(sys.argv[2], sys.argv[3], sys.argv[4], False)

if __name__ == "__main__":
	main()
