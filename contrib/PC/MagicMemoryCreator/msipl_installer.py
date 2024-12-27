#!/usr/bin/env python3
import argparse
import math
import os
import platform
import re
import struct
import subprocess
import time

is_windows = os.name == 'nt'
is_macos = platform.system() == 'Darwin'

MS_IPL_SECTOR_START = 16
BYTES_PER_SECTOR = 512

class Args:
    def __init__(self, device, info=False, insert=str, extract=False, clear=False):
        if is_windows:
            self.pdisk = device
        else:
            self.devname = device
        self.info = info
        self.insert = insert
        self.extract = extract
        self.clear = clear

class MBR_Partition:
    def __init__(self, data):

        (self.status, self.first_head, self.first_sec_clu,
         self.type, self.last_head, self.last_sec_clu, self.LBA, self.sectors) = struct.unpack('<BBHBBHII', data)

    def active(self):
        return self.status != 0

class MBR:
    def __init__(self, data):
        self.code = data[:218]

        (self.drive, self.seconds, self.minutes, self.hours) = struct.unpack('<xxcccc', data[218:224])

        self.code2 = data[224:440]

        (self.signature, self.copy_protection_code) = struct.unpack('<IH', data[440:446])

        self.is_protected = self.copy_protection_code == 0x5A5A

        self.partitions = []

        for i in range(4):
            self.partitions.append(MBR_Partition(data[446+i*16:446+(i+1)*16]))

        (self.boot_signature,) = struct.unpack('<H', data[510:])

def main(args):
    if is_windows:
        diskID = f'\\\\.\\PHYSICALDRIVE{args.pdisk}'

        import wmi

        wmi_h = wmi.WMI()

        def checkDiskType():
            for item in wmi_h.Win32_DiskDrive():
                if item.DeviceID == diskID:
                    isRemovable = item.MediaType == 'Removable Media'
                    isBytesPerSecOk = item.BytesPerSector == BYTES_PER_SECTOR
                    if isRemovable and isBytesPerSecOk:
                        return True

            return False

        def openDisk():
            return open(diskID, 'rb+')

    elif is_macos:
        diskID = f'/dev/{args.devname}'

        def checkDiskType():
            block_size = subprocess.Popen([f"diskutil info {diskID} | awk '/Device Block Size/ {{print $4}}'"], shell=True, stdout=subprocess.PIPE)
            isBytesPerSecOk = bytesPerSector = int(block_size.stdout.read().decode())
            return True

        def openDisk():
            return open(diskID, 'rb+')

    else:
        diskID = f'/dev/{args.devname}'

        def checkDiskType():
            cat = subprocess.run(['cat', f'/sys/block/{args.devname}/queue/hw_sector_size'], stdout=subprocess.PIPE, universal_newlines=True)
            if cat.returncode != 0:
                raise RuntimeError('Selected drive does not exist!')
            bytesPerSector = int(cat.stdout)
            lsblk = subprocess.run(['lsblk', '-dno', 'name,tran,rm'], stdout=subprocess.PIPE, universal_newlines=True)
            for line in lsblk.stdout.splitlines():
                m = re.search('(\w+)\s+(\w+)\s+(\d)', line)
                if m:
                    name, type, rw = m.group(1), m.group(2), m.group(3)
                    if name == args.devname:
                        isRemovable = rw == '1'
                        isBytesPerSecOk = bytesPerSector == BYTES_PER_SECTOR
                        if isRemovable and isBytesPerSecOk:
                            return True
            return False

        def openDisk():
            return open(diskID, 'rb+')

    if checkDiskType() != True:
        raise RuntimeError('Selected drive is not removeable media!')

    with openDisk() as f:
        # Read MBR
        data = f.read(BYTES_PER_SECTOR)
        m = MBR(data)

        if args.info:
            for part in m.partitions:
                if part.active():
                    print(f'Boot Status      - 0x{part.status:02X}')
                    print(f'Start Head       - 0x{part.first_head:02X}')
                    print(f'Start Sec/Clu    - 0x{part.first_sec_clu:02X}')
                    print(f'Partition Type   - 0x{part.type:02X}')
                    print(f'Last Head        - 0x{part.last_head:02X}')
                    print(f'Last Sec/Clu     - 0x{part.last_sec_clu:02X}')
                    print(f'Abs Sector       - 0x{part.LBA:08X}')

            print(f'Signature        - 0x{m.boot_signature:X}')

            data_start_sector = m.partitions[0].LBA

            num_sectors = data_start_sector - MS_IPL_SECTOR_START
            bytes_free = num_sectors * BYTES_PER_SECTOR
            print(f'IPL Section has {bytes_free/1024} KiB free space')
        elif args.extract:
            # Seek to IPL start
            f.seek(MS_IPL_SECTOR_START * BYTES_PER_SECTOR)

            ipl_data = bytearray()

            data_start_sector = m.partitions[0].LBA
            num_sectors = data_start_sector - MS_IPL_SECTOR_START

            # Read all available IPL space
            for i in range(num_sectors):
                data = f.read(BYTES_PER_SECTOR)
                ipl_data += data

            # Get (current) length of IPL data
            length = len(ipl_data)

            # 'Is provided sector empty?' function
            def isEmpty(data):
                for d in data:
                    if d != 0:
                        return False
                return True

            # Reduce the size of extracted IPL data by removing fully empty sectors from the end
            for i in range(num_sectors-1, 0, -1):
                if isEmpty(ipl_data[i*BYTES_PER_SECTOR:(i+1)*BYTES_PER_SECTOR]):
                    length -= BYTES_PER_SECTOR
                else:
                    break

            # Dump IPL data to file
            with open(args.extract, 'wb') as o:
                o.write(ipl_data[:length])
        elif args.insert:
            with open(args.insert, 'rb') as i:
                ipl_data = i.read()

            def alignTo(data, alignment):
                aligned_len = math.ceil(len(ipl_data) / alignment)
                padding_len = aligned_len*alignment - len(ipl_data)
                return data + bytearray(padding_len)

            ipl_data = alignTo(ipl_data, BYTES_PER_SECTOR)

            data_start_sector = m.partitions[0].LBA

            num_sectors = data_start_sector - MS_IPL_SECTOR_START
            bytes_free = num_sectors * BYTES_PER_SECTOR

            if len(ipl_data) > bytes_free:
                raise RuntimeError('Not enough space for the IPL to fit. MS format required.')

            f.seek(MS_IPL_SECTOR_START * BYTES_PER_SECTOR)
            f.write(ipl_data)
        elif args.clear:
            data_start_sector = m.partitions[0].LBA

            num_sectors = data_start_sector - MS_IPL_SECTOR_START
            zero_data = bytearray(num_sectors*BYTES_PER_SECTOR)

            f.seek(MS_IPL_SECTOR_START * BYTES_PER_SECTOR)
            f.write(zero_data)

    if not is_windows:
        subprocess.run(['sync'], stdout=subprocess.PIPE, universal_newlines=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='MS IPL Installer')

    if is_windows:
        parser.add_argument('--pdisk', help='Physical Disk number (WIN32)', type=int, required=True)
    else:
        parser.add_argument('--devname', help='Device name (UNIX)', type=str, required=True)

    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument('--extract', help='Filename to extract MS IPL to',  type=str)
    action.add_argument('--insert', help='Filename of MS IPL to insert', type=str)
    action.add_argument('--clear', help='Clear MS IPL', action='store_true')
    action.add_argument('--info', help='MS info', action='store_true')

    args = parser.parse_args()

    main(args)
