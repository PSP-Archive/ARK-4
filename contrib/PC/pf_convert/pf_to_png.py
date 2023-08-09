#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2023 Ivan Vatlin <jenrus@riseup.net>
import sys  # sys.argv
import zlib # zlib.crc32(); zlib.compress()
# Usage: pf_to_png.py <input_file> <output_file>
# Image specs: PNG, 128x128 pixels, 1 bit grayscale, no Adam7 interlace
def pf_to_png(pf):
    png_signature = b'\x89\x50\x4e\x47\x0d\x0a\x1a\x0a'
    ihdr = (
        b'\x00\x00\x00\x0d' # Chunk data length (13 bytes)
        b'\x49\x48\x44\x52' # IHDR chunk signature
        b'\x00\x00\x00\x80' # width: 128 pixels
        b'\x00\x00\x00\x80' # height: 128 pixels
        b'\x01'             # bit depth: 1 bit per pixel
        b'\x00'             # color type: grayscale
        b'\x00'             # compression method: default
        b'\x00'             # filter method: default
        b'\x00'             # interlace method: no interlace
        b'\xeb\x45\x5c\x66' # CRC32 checksum (zlib.crc32().to_bytes(4, 'big'))
    )
    idat = bytearray()
    idat_signature = b'\x49\x44\x41\x54' # IDAT chunk signature
    iend = (
        b'\x00\x00\x00\x00' # Chunk data length (0 bytes)
        b'\x49\x45\x4e\x44' # IEND chunk signature
        b'\xae\x42\x60\x82' # CRC32 checksum (zlib.crc32().to_bytes(4, 'big'))
    )
    for i in range(0, 2048, 128):
        for j in range(0, 8, 1):
            idat.extend(b'\x00') # add filter type (none) byte
            for k in range(0, 128, 8):
                idat.append(pf[i + j + k])
    idat = zlib.compress(idat)
    idat_length = len(idat).to_bytes(4, 'big')
    idat = idat_signature + idat
    output = png_signature + ihdr + idat_length + idat + zlib.crc32(idat).to_bytes(4, 'big') + iend
    return output

with open(sys.argv[1], mode="rb") as pf_file:
    pf_data = pf_file.read()
    png_file = open(sys.argv[2], mode="wb")
    png_file.write(pf_to_png(pf_data))
