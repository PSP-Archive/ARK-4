#!/usr/bin/env python3

import sys, os, struct, gzip, hashlib
from io import BytesIO

class FakeTime:
    def time(self):
        return 1225856967.109


gzip.time = FakeTime()

def gzipCompress(fn):
    sio=BytesIO()

    with gzip.GzipFile(fileobj=sio, mode='wb') as gz:
        with open(fn, "rb") as f:
            gz.writelines(f)
            osize = f.tell()

    sio.seek(0)
    d = sio.read()
    print ("Original: %d bytes, Compressed: %d bytes, Ratio: %.2f%%" % (osize, len(d), 100.0 * len(d) / osize))

    return d

def main():
    if len(sys.argv) < 3:
        print ("Usage: %s <infile> <outfile>" % (sys.argv[0]))
        sys.exit(1)

    fn = sys.argv[1]
    ofn = sys.argv[2]
    gz = gzipCompress(fn)

    with open(ofn, "wb") as f:
        f.write(gz)

if __name__ == "__main__":
    main()
