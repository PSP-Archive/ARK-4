#!/usr/bin/python3
# Usage: python3 make_satelite_and_pf.py <normal_satelite_path> <bdf_font_path> <encoded_satelite_path> <encoded_pf_font_path>
import sys

def new_pf_map():
    # only 256 characters in pf
    pf_map = [''] * 256

    # do not use \0\r\n, can't display
    pf_map[0] = '\0'
    pf_map[ord('\r')] = '\r'
    pf_map[ord('\n')] = '\n'

    # displayable characters, if necessary, delete a-z or A-Z
    for i in range(6 * 16 - 1):
        pf_map[32 + i] = chr(32 + i)

    return pf_map

def full_pf_map(pf_map: list, satelite_str: str):
    for c in satelite_str:
        if c in pf_map:
            continue

        # hope it doesn't exceed
        pf_map[pf_map.index('')] = c

def need_font_f(pf_map):
    return lambda font_str: chr(int(font_str[0:4], 16)) in pf_map

def paste_font(font_string: str):
    lines = font_string.split('\n')
    in_bitmap = False
    bitmap = b''
    width = 8
    height = 8
    x = 0
    y = 0
    for line in lines:
        if in_bitmap:
            if line.startswith('ENDCHAR'):
                break

            # in QuanPixel, only 45 characters have too large bitmap to be used, they are:
            # ý Ą Ę Ģ Į Ķ ķ Ļ ļ Ņ Ŗ Ş Ţ Ų Ș Ț Ȩ Ḅ Ḏ Ḫ ḫ Ḵ ḵ Ḷ Ḻ ḻ Ṇ Ṉ Ṟ Ṯ Ẕ ẖ Ẹ Ị Ọ Ụ ‱ Ⅶ Ⅷ Ⅻ ⅶ ⅷ ⅻ 〔 〕
            bitmap += (int(line, 16) >> x).to_bytes()

        if line.startswith('BBX '):
            [width, height, x, y] = map(int, line.split()[1:])
            if x < 0:
                x = 0
            elif x > 8 - width:
                x = 8 - width
            if y < 0:
                y = 0
            elif y > 8 - height:
                y = 8 - height
            if 8 - height - y > 1:
                bitmap += b'\0' * (8 - height - y - 1)
        elif line.startswith('BITMAP'):
            in_bitmap = True
    return (chr(int(lines[0], 16)), (bitmap + b'\0\0\0\0\0\0\0\0')[0:8])

normal_satelite_path = sys.argv[1]
bdf_font_path = sys.argv[2]
encoded_satelite_path = sys.argv[3]
encoded_pf_font_path = sys.argv[4]

satelite_str = ''
with open(normal_satelite_path, 'r') as satelite_file:
    satelite_str = satelite_file.read()

pf_map = new_pf_map()
full_pf_map(pf_map, satelite_str)
print(pf_map)

with open(encoded_satelite_path, 'wb') as encoded_satelite_file:
    for c in satelite_str:
        encoded_satelite_file.write(pf_map.index(c).to_bytes())

fonts_data = {}
with open(bdf_font_path, 'r') as bdf_font_file:
    fonts_data = dict(map(paste_font, filter(need_font_f(pf_map), bdf_font_file.read().split('STARTCHAR U+')[1:])))
print(fonts_data)

with open(encoded_pf_font_path, 'wb') as pf_file:
    for c in pf_map:
        pf_file.write(fonts_data[c] if c in fonts_data else b'\0\0\0\0\0\0\0\0')
