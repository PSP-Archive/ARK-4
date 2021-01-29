#!/usr/bin/env python2

from transfer import *
import glob

''' Copy game to /ISO '''
def installGame(fname):
	global cnt

	ftp = FTP(VITA_FTP_HOST, timeout=1)
	ftp.login()
	ftp.set_pasv(False)
	ftp.mkd('/ISO')
	ftp.cwd('/ISO')
	print("Sending the file")

	for fn in glob.glob(fname):
		remote_fn = os.path.split(fn)[-1]
		cut = 0
		ftp.storbinary('STOR ' + remote_fn,  open(fn, 'rb'), callback=callback)
		print ('%s Done' % remote_fn)

	print ('All Done')

def usage():
	print ("Usage: %s <iso>" % sys.argv[0])

def main():
	if len(sys.argv) < 2:
		usage()
		sys.exit(1)

	installGame(sys.argv[1])

if __name__ == "__main__":
	main()
