#!/usr/bin/env python3

import os, sys, struct

SECT_VSH = 1
SECT_GAME = 2
SECT_UPDATER = 4
SECT_POPS = 8
SECT_LICENSE = 0x10
SECT_APP = 0x20
SECT_UMDEMU = 0x40
SECT_MLNAPP = 0x80

TYPE_SECT = 0
TYPE_NOPERCENT = 1
TYPE_PERCENT = 2
TYPE_TWOPERCENT = 4
TYPE_DOLLAR = 0x8000

MODES_DEF = [
        [ SECT_VSH, 2],
        [ SECT_GAME, 1],
        [ SECT_UPDATER, 3],
        [ SECT_POPS, 4],
        [ SECT_LICENSE, 5],
        [ SECT_APP, 6],
        [ SECT_UMDEMU, 7],
        [ SECT_MLNAPP, 8],
]

class ModeEntry:
    fmt = '<HHII20x'
    size = struct.calcsize(fmt)
    def __init__(self, maxsearch, searchstart, modeflag, mode2):
        self.data = struct.pack(self.fmt, maxsearch, searchstart, modeflag, mode2)

class ModuleEntry:
    def __init__(self, stroffset, flags):
        self.data = struct.pack('<IIII16x', stroffset, 0, flags, 0);
    def load(self, data):
        s = struct.Struct('<IIII16x')
        v = s.unpack(data[0:s.size])
        self.stroffset = v[0]
        self.flags = v[2]

class BtcnfHeader:
    def __init__(self, devkit, modestart, nmodes, modulestart, nmodules, modnamestart, modnameend):
        self.data = struct.pack('<16I', 0x0F803001, devkit, 0x6B8B4567, 0x327B23C6, modestart, nmodes, \
                0x643C9869, 0x66334873, modulestart, nmodules, 0x74B0DC51, 0x19495CFF, \
                modnamestart, modnameend, 0x2AE8944A, 0x625558EC)
    def load(self, data):
        s = struct.Struct('<16I')
        v = s.unpack(data[0:s.size])
        self.magic = v[0]
        self.devkit = v[1]
        self.modestart = v[4]
        self.nmodes = v[5]
        self.modulestart = v[8]
        self.nmodules = v[9]
        self.modnamestart = v[12]
        self.modnameend = v[13]

class Module:
    def __init__(self, path, flags):
        self.path = path
        self.flags = flags
        self.stroffset = 0

' Remove duplication in a list '
def trimList(srcList):
    newList = []
    i = 0
    
    while i < len(srcList):
        if srcList[i] not in newList:
            newList.append(srcList[i])
        i += 1

    return newList

def getModuleString(data, pos):
    s = []

    while pos < len(data):
        if chr(data[pos]) != '\x00':
            s.append(chr(data[pos]))
        else:
            break

        pos += 1

    
    return "".join(s).replace('\x00', '\n')

class pspBtCnf:
    def __init__(self):
        self.module_list = []
        self.fw_version = 0
    def loadBin(self, fn):
        with open(fn, "rb") as f:
            data = f.read()

            hdr = BtcnfHeader(0, 0, 0, 0, 0, 0, 0)
            hdr.load(data)

            if hdr.magic != 0x0F803001:
                raise RuntimeError("Not a pspbtcnf file did you decrypt it first?")
            self.fw_version = hdr.devkit

            i = 0

            while i < hdr.nmodules:
                pos = hdr.modulestart + i * ModeEntry.size
                med = data[pos:pos + ModeEntry.size]
                me = ModuleEntry(0, 0)
                me.load(med)
                off = hdr.modnamestart + me.stroffset
                module_path = getModuleString(data, off)
                mod = Module(module_path, me.flags)
                self.module_list.append(mod)

                i += 1


    def parseLine(self, line):
        btmode = 0

        if line.startswith('#'):
            return

        line = line.strip()
        flags = 0

        if line.startswith('$%%'):
            flags = 0x8004
        elif line.startswith('$%'):
            flags = 0x8002
        elif line.startswith('$'):
            flags = 0x8001
        elif line.startswith('%%'):
            flags = 4
        elif line.startswith('%'):
            flags = 2
        else:
            flags = 1

        module_path, btmode_str = line.split(' ')[0:2]
        btmode_str  = btmode_str.upper()

        if 'V' in btmode_str:
            btmode |= SECT_VSH
        if 'G' in btmode_str:
            btmode |= SECT_GAME
        if 'U' in btmode_str:
            btmode |= SECT_UPDATER
        if 'P' in btmode_str:
            btmode |= SECT_POPS
        if 'L' in btmode_str:
            btmode |= SECT_LICENSE
        if 'A' in btmode_str:
            btmode |= SECT_APP
        if 'E' in btmode_str:
            btmode |= SECT_UMDEMU
        if 'M' in btmode_str:
            btmode |= SECT_MLNAPP

        module_path = module_path[module_path.find("/"):]

        mod = Module(module_path, (flags << 16 | btmode))
        self.module_list.append(mod)

    def loadText(self, fn):
        with open(fn, "r") as f:
            line = f.readline().strip()

            while line.startswith('#'):
                line = f.readline().strip()

            self.fw_version = int(line, 16)

            while True:
                line = f.readline()

                if len(line) == 0:
                    break
                
                self.parseLine(line)

    def writeModuleEntry(self, f, modulestart):
        f.seek(modulestart)

        for mod in self.module_list:
            me = ModuleEntry(mod.stroffset, mod.flags)
            f.write(me.data)

    def outputText(self, fn):
        with open(fn, "w") as f:
            f.write("0x%08X\n" % (self.fw_version))
            i=0
            for mod in self.module_list:
                loadmode = mod.flags >> 16

                if loadmode & TYPE_DOLLAR:
                    f.write('$')
                if loadmode & TYPE_PERCENT:
                    f.write('%')
                if loadmode & TYPE_TWOPERCENT:
                    f.write('%%')
                

                f.write(mod.path)
                f.write(" ")

                if mod.flags & SECT_VSH:
                    f.write('V')
                if mod.flags & SECT_GAME:
                    f.write('G')
                if mod.flags & SECT_UPDATER:
                    f.write('U')
                if mod.flags & SECT_POPS:
                    f.write('P')
                if mod.flags & SECT_LICENSE:
                    f.write('L')
                if mod.flags & SECT_APP:
                    f.write('A')
                if mod.flags & SECT_UMDEMU:
                    f.write('E')
                if mod.flags & SECT_MLNAPP:
                    f.write('M')

                f.write("\n")

    def isModeUsed(self, mode):
        for mod in self.module_list:
            if mode & mod.flags:
                return True

        return False

    def getUsedMode(self):
        usedMode = []

        for predefinedMode in MODES_DEF:
            if self.isModeUsed(predefinedMode[0]):
                usedMode.append(predefinedMode)

        return usedMode

    def updateModuleStrOffset(self, path, stroffset):
        for mod in self.module_list:
            if mod.path == path:
                mod.stroffset = stroffset
    
    def writeHeader(self, fp, modestart, nmode, modulestart, nmodules, modnamestart, modulesend):
        hdr = BtcnfHeader(self.fw_version, modestart, nmode, modulestart, nmodules, modnamestart, modulesend)
        fp.write(hdr.data)

    def writeMode(self, fp, nmodules):
        modes = self.getUsedMode()
        nmode = len(modes)
        modestart = fp.tell()

        for mode in modes:
            me = ModeEntry(nmodules, 0, mode[0], mode[1])
            fp.write(me.data)

        return nmode, modestart

    def writeModuleString(self, fp, modnamestart):
        modstrings = [ mod.path for mod in self.module_list ]
        modstrings = trimList(modstrings)

        for modstr in modstrings:
            self.updateModuleStrOffset(modstr, fp.tell() - modnamestart)
            fp.write(modstr.encode())
            fp.write('\x00'.encode())

    def outputBin(self, fn):
        with open(fn, "wb") as f:
            nmodules = len(self.module_list)
            self.writeHeader(f, 0, 0, 0, 0, 0, 0)
            nmode, modestart = self.writeMode(f, nmodules)
            modulestart = f.tell()
            self.writeModuleEntry(f, modulestart)
            modnamestart = f.tell()
            self.writeModuleString(f, modnamestart)
            modulesend = f.tell()
            f.seek(0)
            self.writeHeader(f, modestart, nmode, modulestart, nmodules, modnamestart, modulesend)
            self.writeModuleEntry(f, modulestart)

def usage():
    print ("Usage: %s <build|extract> <btcnf.bin|btcnf.txt>" % (sys.argv[0]))

def main():
    if len(sys.argv) < 3:
        usage()
        sys.exit(1)

    cmd = sys.argv[1]
    fn = sys.argv[2]
    btcnf = pspBtCnf()

    if cmd.lower() == "build":
        nfn = os.path.splitext(fn)[0] + ".bin"
        btcnf.loadText(fn)
        btcnf.outputBin(nfn)
        print("%s done" % nfn)
    elif cmd.lower() == "extract" or cmd.lower() == "ext":
        nfn = os.path.splitext(fn)[0] + ".txt"
        btcnf.loadBin(fn)
        btcnf.outputText(nfn)
        print("%s done" % nfn)
    else:
        usage()
        sys.exit(1)
    
if __name__ == "__main__":
    main()
