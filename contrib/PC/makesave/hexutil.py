#!/usr/bin/env python2

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

def test():
	assert(toHex(toBinary("01434430303101005053502047414D45")) == "01434430303101005053502047414D45")

if __name__ == "__main__":
	test()
