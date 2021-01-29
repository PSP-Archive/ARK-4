#!/usr/bin/env python2

import sys, subprocess, StringIO, re

''' Script to generate dependency for c source '''

def runCmd(cmd):
	f = subprocess.Popen(cmd, shell=False, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
	stdout, stderr = f.communicate()
	retcode = f.wait()

	return {'stdout':StringIO.StringIO(stdout), 'stderr':StringIO.StringIO(stderr), 'retcode': retcode}

def processDep(fp, ofn):
	res = StringIO.StringIO()

	for line in fp:
		' Add depend file in target'
		m = re.sub(r'^(.*?\.o)(:)', r'\1 %s\2' % ofn, line)
		res.write(m)
	
	res.seek(0)
	return res

def usage():
	print ("Usage: %s <outputfile> [gcc arguments]" % (sys.argv[0]))

def main():
	if len(sys.argv) < 3:
		usage()
		sys.exit(1)

	ofn = sys.argv[1]
	gcc_args = sys.argv[2:]
	res = runCmd(gcc_args)

	if res['retcode'] != 0:
		sys.exit(res['retcode'])

	with open(ofn, "wb") as of:
		dep = res['stdout']
		dep = processDep(dep, ofn)
		of.write(dep.read())
	
	stderr = res['stderr'].read()

	if len(stderr):
		sys.stderr.write(stderr)

if __name__ == "__main__":
	main()
