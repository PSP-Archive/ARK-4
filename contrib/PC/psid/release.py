#!/usr/bin/python

import sys
from pspencrypt import toBinary, toHex

def main():
	if len(sys.argv) < 2:
		sys.exit(1)

	psid = sys.argv[1]
	
	print "// " + psid
	print "#define RELEASE_KEY {",

	i = 0

	psid = toBinary(psid)
	psid = toHex(psid)

	mot = "0x%s,"
	while i < len(psid):
		if i == len(psid)-2:
			mot = "0x%s"
		print mot % (psid[i:i+2]),
		i += 2

	print "}";

if __name__ == "__main__":
	main()
