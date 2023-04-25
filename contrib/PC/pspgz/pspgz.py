#!/usr/bin/env python3

class FakeTime:
    def time(self):
        return 1225856967.109

import sys, os, struct, gzip, hashlib
from io import BytesIO

gzip.time = FakeTime()

def binary_replace(data, newdata, offset):
    return data[0:offset] + newdata + data[offset+len(newdata):]

def prx_compress(output, hdr, input, mod_name="", mod_attr=0xFFFFFFFF):
    a=open(hdr, "rb")
    fileheader = a.read();
    a.close()

    a=open(input, "rb")
    elf = a.read(4);
    a.close()

    if (elf != '\x7fELF'.encode()):
        print ("not a ELF/PRX file!")
        return -1

    uncompsize = os.stat(input).st_size

    f_in=open(input, 'rb')
    temp=BytesIO()
    f=gzip.GzipFile(fileobj=temp, mode='wb')
    f.writelines(f_in)
    f.close()
    f_in.close()
    prx=temp.getvalue()
    temp.close()

    digest=hashlib.md5(prx).digest()
    filesize = len(fileheader) + len(prx)

    if mod_name != "":
        if len(mod_name) < 28:
            mod_name += "\x00" * (28-len(mod_name))
        else:
            mod_name = mod_name[0:28]
        fileheader = binary_replace(fileheader, mod_name.encode(), 0xA)

    if mod_attr != 0xFFFFFFFF:
        fileheader = binary_replace(fileheader, struct.pack('H', mod_attr), 0x4)

    fileheader = binary_replace(fileheader, struct.pack('L', uncompsize), 0x28)
    fileheader = binary_replace(fileheader, struct.pack('L', filesize), 0x2c)
    fileheader = binary_replace(fileheader, struct.pack('L', len(prx)), 0xb0)
    fileheader = binary_replace(fileheader, digest, 0x140)

    a=open(output, "wb")
    assert(len(fileheader) == 0x150)
    a.write(fileheader)
    a.write(prx)
    a.close()

    try:
        os.remove("tmp.gz")
    except OSError:
        pass

    return 0

def main():
    if len(sys.argv) < 4:
        print ("Usage: %s outfile prxhdr infile [modname] [modattr]\n"%(sys.argv[0]))
        exit(-1)

    if len(sys.argv) < 5:
        prx_compress(sys.argv[1], sys.argv[2], sys.argv[3])
    elif len(sys.argv) < 6:
        prx_compress(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        prx_compress(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], int(sys.argv[5], 16))

if __name__ == "__main__":
    main()
