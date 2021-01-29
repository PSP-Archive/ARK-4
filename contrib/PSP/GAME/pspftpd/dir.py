#!/usr/bin/env python2

from transfer import *

''' Copy dirname to /PSP/SAVEDATA '''
def installDir(dirname):
	global cnt

	ftp = FTP(VITA_FTP_HOST, timeout=1)
	ftp.login()
	ftp.set_pasv(False)
	vita_dirname = '/PSP/SAVEDATA/%s' % dirname
	ftp.mkd(vita_dirname)
	ftp.cwd(vita_dirname)
	print("Sending the file")

	for root, dirs, files in os.walk(dirname):
		for fname in files:
			full_fname = os.path.join(root, fname)
			cut = 0
			ftp.storbinary('STOR ' + fname,  open(full_fname, 'rb'), callback=callback)
			print ('%s Done' % fname)
	
	print ('All Done')

def usage():
	print ("Usage: %s <iso>" % sys.argv[0])

def main():
	if len(sys.argv) < 2:
		usage()
		sys.exit(1)

	installDir(sys.argv[1])

if __name__ == "__main__":
	main()
