#! /usr/bin/env python

import sys
from pspencrypt import psidHash, toBinary, toHex

def main():
	if len(sys.argv) < 3:
		sys.exit(1)
	
	sys.stdout = open(sys.argv[2], "w")

	print "unsigned char psidHash[16] = "
	print "{"
	print "\t",

	i = 0

	psid = sys.argv[1]
	psid = toBinary(psid)
	psid = psidHash(psid)
	psid = toHex(psid)

	while i < len(psid):
		if i == 2 * 8:
			print ""
			print "\t",
		print "0x%s, " % (psid[i:i+2]),
		i += 2

	print ""
	print "};";

if __name__ == "__main__":
	main()
