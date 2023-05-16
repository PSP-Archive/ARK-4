import psptool.prx as prx
import psptool.kirk as kirk
import struct
import zlib
import csv
import io
import math

import hashlib

from Crypto.Cipher import DES
from Crypto.Util.strxor import strxor as xor

_psar_table_keys = [
    {
        'key': '95620B49B730E5C7',
        'iv': '9EA43381860C5285'
    },
    {
        'key': '5A7B3D9D45C9DC95',
        'iv': 'B2FED9798A02B187'
    },
    {
        'key': '4CCE495B6F20585A',
        'iv': '8108C1F2359869B0'
    },
    {
        'key': '73F45262620BF15A',
        'iv': '6D521BA3C236F92B'
    },
    {
        'key': 'A664C8F8FD9D4498',
        'iv': 'DB4E7941F59730AD'
    },
    {
        'key': 'D7BD74813D6426E7',
        'iv': 'A6830C2F630B9629'
    }
]


def decrypt_table(f, table, mode):
    des = DES.new(bytes.fromhex(_psar_table_keys[mode]['key']), DES.MODE_CBC, iv=bytes.fromhex(
        _psar_table_keys[mode]['iv']))
    return prx.decrypt(des.decrypt(table))


def is_table(name, version):
    if version == 1 or version == 2:
        return name == '01g:00000' or name == '02g:00000'
    elif version == 3 or version == 4:
        return int(name) < 10
    return False


def table_model(name, version):
    if version == 1 or version == 2:
        if name.startswith('01g'):
            return 0
        elif name.startswith('02g'):
            return 1
    elif version == 3 or version == 4:
        return int(name)
    return 0


class psar_file(object):
    def __init__(self, data):
        self.name = struct.unpack('128s', data[4:0x84])[
            0].decode().split('\0')[0]
        (self.size, self.expanded_size, self.flags) = struct.unpack(
            '<III', data[0x104:0x110])


class PSAR(object):
    def __init__(self, data):
        self._version = struct.unpack('<I', data[4:8])[0]
        metadata = self._decrypt_block(data[0x10:0x270])
        second_size = struct.unpack('<I', metadata[0xC:0x10])[0]
        info = struct.unpack('<64s', metadata[0x10:0x50])
        info = b''.join(info).decode('utf-8').rstrip('\x00')
        version = info.split(',')[5]

        if version.startswith('3.8') or version.startswith('3.9'):
            table_version = 1
            delimiter = '|'
        elif version.startswith('4.'):
            table_version = 2
            delimiter = '|'
        elif version.startswith('5.'):
            table_version = 3
            delimiter = ','
        elif version.startswith('6.'):
            table_version = 4
            delimiter = ','
        else:
            table_version = 0
            delimiter = ','

        if self._version != 1:
            block = self._decrypt_block(data[0x270:0x270+second_size])

        files = {}
        pos = 0x270+second_size

        while pos < len(data)-0x10:
            size = self._get_file_entry_size(data[pos:pos+0x260])
            f = self._read_file(data[pos:pos+size])
            files[f['name']] = f
            pos += size

        tables = {table_model(f, table_version): decrypt_table(f,
                                                               files[f]['file'], table_version).decode() for f in files if is_table(f, table_version)}
        # print(tables)

        # print(table)
        # f3 = io.StringIO(table)
        # reader = csv.reader(f3, delimiter=delimiter)
        # if table_key == 2:
        #     table_files = {x[0]: x[1] for x in reader}
        # else:
        #     table_files = {x[0]: x[1] for x in reader}

        # print(files.keys())

        table_files = {}

        for x in tables:
            if table_version == 1 or table_version == 2:
                if x == 0:
                    model = '01g:'
                else:  # x == 1:
                    model = '02g:'

                f3 = io.StringIO(tables[x])
                reader = csv.reader(f3, delimiter=delimiter)
                for x in reader:
                    if model + x[0] in files:
                        table_files[model + x[0]] = x[1]
                    else:
                        table_files['com:' + x[0]] = x[1]
            else:
                f3 = io.StringIO(tables[x])
                reader = csv.reader(f3, delimiter=delimiter)
                for x in reader:
                    table_files[x[0]] = x[1]

        files = [{'name': table_files[x], 'data':files[x]}
                 for x in table_files]
        self.directories = [f['name']
                            for f in files if f['data']['file'] is None]
        self.directories.sort()
        self.files = [f for f in files if f['data']['file'] is not None]

    def _demangle(self, block):
        if self._version == 5:
            block = xor(block, math.ceil(
                len(block)/0x10)*bytes.fromhex('D869B895336B633498B9FC3CB7262BD7')[:len(block)])

        block = kirk.kirk7(block, 0x55)

        if self._version == 5:
            block = xor(block, math.ceil(
                len(block)/0x10)*bytes.fromhex('0DA09084AF9EB6E2D294F2AAEF996871')[:len(block)])

        return block

    def _decrypt_block(self, block):
        if self._version != 1:
            block = block[:0x20] + \
                self._demangle(block[0x20:0x150]) + block[0x150:]
        return prx.decrypt(block)

    def _get_file_entry_size(self, data):
        meta = psar_file(self._decrypt_block(data))
        return 0x260+meta.size

    def _read_file(self, data):
        meta = psar_file(self._decrypt_block(data))
        file = zlib.decompress(self._decrypt_block(
            data[0x260:0x260+meta.size])) if meta.expanded_size > 0 else None
        return {"name": meta.name, "flags": meta.flags, "file": file}
