#!/usr/bin/python3

import os, sys, getopt

def usage():
	print ("Usage: %s [-l size ] basefile input output" % (sys.argv[0]))

def write_file(fn, buf):
	fp = open(fn, "wb")
	fp.write(buf)
	fp.close()

def main():
	inputsize = 0
	basefilelen = 0x1000
	maxinputsize = 0x4000

	try:
		optlist, args = getopt.getopt(sys.argv[1:], 'l:hk')
	except getopt.GetoptError:
		usage()
		sys.exit(2)

	for o, a in optlist:
		if o == "-h":
			usage()
			sys.exit()
		if o == "-l":
			inputsize = int(a, 16)
		if o == "-k":
			basefilelen = 0x2000
			maxinputsize = 0x5000
	
	inputsize = max(inputsize, maxinputsize);

	if len(args) < 3:
		usage()
		sys.exit(2)
	
	basefile = args[0]
	inputfile = args[1]
	outputfile = args[2]

	fp = open(basefile, "rb")
	buf = fp.read(basefilelen)
	fp.close()

	if len(buf) < inputsize:
		buf += ('\0' * (inputsize - len(buf))).encode()

	assert(len(buf) == inputsize)

	fp = open(inputfile, "rb")
	ins = fp.read(0x3000)
	fp.close()
	buf = buf[0:basefilelen] + ins + buf[basefilelen+len(ins):]
	assert(len(buf) == inputsize)
	write_file(outputfile, buf)

if __name__ == "__main__":
	main()
