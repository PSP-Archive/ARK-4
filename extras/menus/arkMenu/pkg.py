#!/usr/bin/env python3

import os, sys

SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__))
THEMES_DIR = os.path.join(SCRIPT_DIR, 'themes')
THEMES = os.listdir(THEMES_DIR)


def int2bytes(number):
    hex = '%08X' %(number)

    A = int('0x' + hex[6:8], 16).to_bytes(1, 'little')
    B = int('0x' + hex[4:6], 16).to_bytes(1, 'little')
    C = int('0x' + hex[2:4], 16).to_bytes(1, 'little')
    D = int('0x' + hex[0:2], 16).to_bytes(1, 'little')

    return A + B + C + D

for theme in THEMES:
    print("Theme: {}".format(theme))

    THEME_DIR = os.path.join(THEMES_DIR, theme)
    RES_DIR = os.path.join(THEME_DIR, 'resources')
    dir = os.listdir(RES_DIR)
    dir.sort()

    DEST_FILE = os.path.join(THEME_DIR, "THEME.ARK")

    track = []
    with open(DEST_FILE, 'wb+') as pkg:
        for item in dir:
            track.append(pkg.tell())
            pkg.write(int2bytes(0) + int2bytes(len(item)))
            pkg.write(bytes(item + '\0', "UTF8"))

        pkg.write(bytes.fromhex("FFFFFFFF"))

        track.reverse()

        for item in dir:
            cur = pkg.tell()
            pkg.seek(track.pop())
            pkg.write(int2bytes(cur))
            pkg.seek(cur)

            with open(os.path.join(RES_DIR, item), 'rb') as fd:
                pkg.write(fd.read())
