#! /usr/bin/env python

import os

EXTS = ['.c', '.h', '.py', '.cpp']
TAB_SIZE = 4

def isCode(f):
    for e in EXTS:
        if f.endswith(e): return True
    return False

def findtabs(path):

    if not isCode(path): return

    print "Processing: " + path
    
    f = open(path, "r")
    data = f.read()
    f.close()

    data = data.replace('\t', ' '*TAB_SIZE)

    f = open(path, "w")
    f.write(data)
    f.close()

def recursiveFind(path):
    
    root, dirs, files = os.walk(path).next()

    for i in dirs+files:
        if os.path.isdir(root+os.sep+i):
            if not i.startswith("."):
                recursiveFind(root+os.sep+i)
        else:
            findtabs(root+os.sep+i)

recursiveFind(os.getcwd())
