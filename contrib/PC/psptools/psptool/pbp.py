import struct

PBP_HEADER_MAGIC = 0x50425000


def is_pbp(data):
    return struct.unpack('<I', data[:0x4])[0] == PBP_HEADER_MAGIC


class PBP:
    def __init__(self, data):
        (self.magic, self.version, sfo_offset, icon0_offset,
         icon1_offset, pic0_offset, pic1_offset,
         snd0_offset, prx_offset, psar_offset) = struct.unpack('<IIIIIIIIII', data[:0x28])

        self.sfo = data[sfo_offset:icon0_offset]
        self.icon0 = data[icon0_offset:icon1_offset]
        self.icon1 = data[icon1_offset:pic0_offset]
        self.pic0 = data[pic0_offset:pic1_offset]
        self.pic1 = data[pic1_offset:snd0_offset]
        self.snd0 = data[snd0_offset:prx_offset]
        self.prx = data[prx_offset:psar_offset]
        self.psar = data[psar_offset:]

    def pack(self):
        return struct.pack('<IIIIIIIIII', self.magic, self.version,
                           0x28,
                           0x28+len(self.sfo),
                           0x28+len(self.sfo)+len(self.icon0),
                           0x28+len(self.sfo)+len(self.icon0)+len(self.icon1),
                           0x28+len(self.sfo)+len(self.icon0) +
                           len(self.icon1)+len(self.pic0),
                           0x28+len(self.sfo)+len(self.icon0)+len(self.icon1) +
                           len(self.pic0)+len(self.pic1),
                           0x28+len(self.sfo)+len(self.icon0)+len(self.icon1) +
                           len(self.pic0)+len(self.pic1)+len(self.snd0),
                           0x28+len(self.sfo)+len(self.icon0)+len(self.icon1) +
                           len(self.pic0)+len(self.pic1) +
                           len(self.snd0)+len(self.prx)
                           ) + self.sfo + self.icon0 + self.icon1 + self.pic0 + self.pic1 + self.snd0 + self.prx + self.psar
