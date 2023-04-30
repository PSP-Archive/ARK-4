#! /usr/bin/env python3

import os, sys

def make_iso():
    os.system("mkisofs -o ark.iso disc0/")

def sign_iso():
    fd = open("license.rif")
    fd.seek(0x10)
    
    os.system("sign_np -pbp ark.iso EBOOT.PBP "+fd.read(0x23)+" lisense.rif disc0/psp_game/icon0.png")
    
    fd.close()

if __name__ == "__main__":
    make_iso()
    sign_iso()
