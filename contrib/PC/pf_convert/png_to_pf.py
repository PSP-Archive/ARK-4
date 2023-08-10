#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2023 Ivan Vatlin <jenrus@riseup.net>
import sys # sys.argv
import zlib # zlib.decompress()
# Usage: png_to_pf.py <input_file> <output_file>
def png_to_pf(png):
    if png[0:8] == b'\x89\x50\x4e\x47\x0d\x0a\x1a\x0a': # Check PNG signature
        idat_length = int.from_bytes(png[33:37], "big") # Skip IHDR chunk & reading IDAT chunk data length
        idat_zlib = png[41:41+idat_length]              # Read compressed IDAT chunk data
        idat_data = zlib.decompress(idat_zlib)          # Decompress IDAT chunk data
        pf = bytearray()
        for i in range(0, 2176, 136):
            for k in range(0, 16, 1):
                for j in range(1, 136, 17): # Skip filter type byte
                    pf.append(idat_data[i + j + k])
        return pf
    else:
        print('not a PNG file')

with open(sys.argv[1], mode="rb") as png_file:
    png_data = png_file.read()
    pf_file = open(sys.argv[2], mode="wb")
    pf_file.write(png_to_pf(png_data))
