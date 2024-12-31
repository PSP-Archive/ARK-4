#!/usr/bin/env python3

import struct, sys, os

no_delete = False

def readConfig(configPath):
    res = []
    with open(configPath, "r") as f:
        while True:
            l = f.readline()

            if len(l) == 0:
                break

            if l[0] == '#':
                continue

            l = l.strip()

            try:
                vpath,fpath = l.split(',')
                res.append([vpath, fpath])
            except ValueError as e:
                pass

    return res

def pack(outputFile, fileList):
    fileCount = 0
    with open(outputFile, "wb") as of:
        of.write(struct.pack('<L', 0))
        for r in fileList:
            print('Adding %s as %s' % (r[1], r[0]))
            with open(r[1], "rb") as inf:
                fileContent = inf.read()
                fileSize = len(fileContent)
                d = struct.pack('<L', fileSize)

                ' filesize '
                of.write(d)
                ' filename '
                of.write(struct.pack('<B', len(r[0])))
                of.write(r[0].encode())
                ' file content '
                of.write(fileContent)
                fileCount += 1
            try:
                if not no_delete:
                    os.remove(r[1])
            except:
                pass
        of.seek(0)
        of.write(struct.pack('<L', fileCount))

def unpack(inputFile):
    with open(inputFile, "rb") as f:
        f.seek(4)
        while True:
            d = f.read(4)

            if len(d) == 0:
                break

            fileSize = struct.unpack('<L', d)[0]

            d = f.read(1)

            if len(d) == 0:
                break

            lenOfFilename = struct.unpack('<B', d)[0]

            filename = f.read(lenOfFilename)

            if len(filename) == 0:
                break

            fileContent = f.read(fileSize)

            if len(fileContent) == 0:
                break

            savFilename = os.path.split(filename)[-1]

            with open(savFilename, "wb") as of:
                of.write(fileContent)

            print ("Saved %s as %s" % (filename, savFilename))

def usage():
    print ("Usage: %s <mode> args..." % (sys.argv[0]))
    print (" mode: -e <input> : extract all modules from pack")
    print (" mode: -p <output filename> <list> : pack")

def main():
    if len(sys.argv) < 2:
        usage()
        sys.exit(1)

    mode = sys.argv[1]

    if mode == "-p":
        if len(sys.argv) < 4:
            usage()
            sys.exit(1)
        
        print(sys.argv)
        
        if len(sys.argv) == 5 and sys.argv[4] == "-s":
            global no_delete
            no_delete = True
        
        outputFile, fileListConfig = sys.argv[2], sys.argv[3]
        res = readConfig(fileListConfig)
        pack(outputFile, res)
    elif mode == "-e":
        if len(sys.argv) < 3:
            usage()
            sys.exit(1)
        
        inputFile = sys.argv[2]
        unpack(inputFile)
    else:
        usage()
        sys.exit(1)

if __name__ == "__main__":
    main()
