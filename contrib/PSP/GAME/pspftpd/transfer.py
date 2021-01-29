#!/usr/bin/env python2

from ftplib import FTP
import os, time, sys

# the IP of vita please
VITA_FTP_HOST = os.getenv('VITA_FTP_HOST', "192.168.1.104")
# provita dir
DIST = os.getenv('VITA_DIST', 'dist')
# dir to your game savedata
VITA_GAME_PATH = os.getenv('VITA_GAME_PATH', '/PSP/SAVEDATA/UCJS101040000')
# for callback display
DISPLAY_SIZE = 1024 * 1024

oldTime = time.time()
cnt = 0

def callback(s):
	global oldTime, cnt
	cnt += len(s)
	curTime = time.time()
	delta = curTime - oldTime

	if cnt >= DISPLAY_SIZE and delta != 0:
		print >> sys.stderr, ("%.2fKB/s" % (cnt / delta / 1000.0))
		oldTime = curTime
		cnt = 0

def installProVita():
	global cnt

	ftp = FTP(VITA_FTP_HOST, timeout=1)
	ftp.login()
	ftp.set_pasv(False)
	ftp.cwd(VITA_GAME_PATH)
	print("Sending the file")

	for root, dirs, files in os.walk(DIST):
		for fname in files:
			full_fname = os.path.join(root, fname)
			cut = 0
			ftp.storbinary('STOR ' + fname,  open(full_fname, 'rb'), callback=callback)
			print ('%s Done' % fname)
	
	print ('All Done')

def getFile(fname):
	ftp = FTP(VITA_FTP_HOST, timeout=1)
	ftp.login()
	ftp.set_pasv(False)
	ftp.cwd(VITA_GAME_PATH)
	print ("Downloading %s" % (fname))
	ftp.retrbinary('RETR ' + fname,  open(fname, 'wb').write)
	print ("%s done" % (fname))

def deleteFile(fname):
	ftp = FTP(VITA_FTP_HOST, timeout=1)
	ftp.login()
	ftp.set_pasv(False)
	ftp.cwd(VITA_GAME_PATH)
	print ("Deleting %s" % (fname))
	ftp.delete(fname)
	print ("%s done" % (fname))

def main():
	getFile('log.txt')
	installProVita()
	deleteFile('log.txt')

if __name__ == "__main__":
	main()
