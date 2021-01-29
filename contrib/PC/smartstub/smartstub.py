#!/usr/bin/env python2

import struct, sys

'''
		STUB_ENTRY(KernelFreePartitionMemory, SysMemUserForUser, 0xB6D61D02),

		STUB_ENTRY(DisplaySetFrameBuf, sceDisplay, 0x289D82FE),
'''
def lookupName(name):
	d = {
			"IoFileMgrForUser_109F50BC": "IoOpen", 
			"IoFileMgrForUser_6A638D83": "IoRead", 
			"IoFileMgrForUser_42EC03AC": "IoWrite", 
			"IoFileMgrForUser_810C4BC3": "IoClose", 

			"UtilsForUser_27CC57F0": "KernelLibcTime",
			"UtilsForUser_79D1C3FA": "KernelDcacheWritebackAll",

			"ThreadManForUser_446D8DE6": "KernelCreateThread",
			"ThreadManForUser_CEADEB47": "KernelDelayThread",
			"ThreadManForUser_F475845D": "KernelStartThread",
			"ThreadManForUser_278C0DF5": "KernelWaitThreadEnd",
			"ThreadManForUser_89B3D48C": "KernelDeleteVpl",
			"ThreadManForUser_ED1410E0": "KernelDeleteFpl",

			"sceUtility_2A2B3DE0": "UtilityLoadModule",
			"sceUtility_E49BFE92": "UtilityUnloadModule",
			"sceUtility_1579A159": "UtilityLoadNetModule",
			"sceUtility_64D50C56": "UtilityUnloadNetModule",

			"SysMemUserForUser_B6D61D02": "KernelFreePartitionMemory",

			"sceDisplay_289D82FE" : "DisplaySetFrameBuf",
	}
	return d[name] if name in d else name

def main():
	filename = "smart.log" if len(sys.argv) < 2 else sys.argv[1]
	with open(filename, "rb") as f:
		while True:
			strLen = f.read(4)

			if len(strLen) == 0:
				break
			
			strLen = struct.unpack('<L', strLen)[0]
			libname = f.read(strLen)
			nid = f.read(4)
			nid = struct.unpack('<L', nid)[0]
			addr = f.read(4)
			addr = struct.unpack('<L', addr)[0]

			funcName = "%s_%08X" % (libname, nid)

			print "tbl->%s = (void*) 0x%08X;" % (lookupName(funcName), addr)

if __name__ == "__main__":
	main()
