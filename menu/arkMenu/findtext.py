#!/usr/bin/env python2

import os

def findtext(path, text):

	f = open(path, "r")

	lines = f.readlines()

	count = 0

	for i in range(0, len(lines)):
		if text in lines[i]:
			print "found match in file", path, "in line", i+1
			if replace:
				lines[i] = lines[i].replace(text, replace_with)
	f.close()
	if replace:
		f = open(path, "w")
		for i in lines:
			f.write(i)
		f.close()

def recursiveFind(path, text):
	
	root, dirs, files = os.walk(path).next()

	for i in dirs+files:
		if os.path.isdir(root+os.sep+i):
			recursiveFind(root+os.sep+i, text)
		else:
			findtext(root+os.sep+i, text)

replace = raw_input("replace? y/n > ") == "y"
if replace:
	replace_with = raw_input("replace with > ")
text = raw_input("string to find > ")
recursiveFind(os.getcwd(), text)
