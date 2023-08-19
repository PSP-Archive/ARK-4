#!/usr/bin/env python3

import sys

def xor(buf ,val):
    res = bytes()
    for i in range(len(buf)):
        res += bytes([buf[i] ^ val])
    return res

if len(sys.argv) != 4:
    print(f"Usage: {sys.argv[0]} cipl.bin ipl.bin final.bin")
    

with open(sys.argv[1], "rb") as cipl:
    with open(sys.argv[2], "rb") as ipl:
        with open(sys.argv[3], "wb") as dest:
            cipl_data = cipl.read()
            padding = ( '\0' * ( 0x1000 - (len(cipl_data)%0x1000) ) ).encode()
            cipl_data += padding
            
            assert(len(cipl_data)%0x1000 == 0)
            
            ipl_data = ipl.read()
            ipl_size = len(ipl_data) - 0x1000
            ipl_hash = ipl_data[ipl_size:]
            ipl_data = ipl_data[:ipl_size]
            
            assert(len(ipl_hash)==0x1000)
            assert(len(ipl_data)>0)
            
            #cipl_data = xor(cipl_data, 1)
            
            dest.write(ipl_data)
            dest.write(cipl_data)
            dest.write(cipl_data)
            dest.write(ipl_hash)

