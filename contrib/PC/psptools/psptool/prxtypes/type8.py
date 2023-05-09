import psptool.kirk as kirk

from Crypto.Util.strxor import strxor as xor
from Crypto.Hash import SHA1
from Crypto import Random


class prx_header_8(object):
    def __init__(self, header=None):
        if header is None:
            self.header = b'\x00'*0x150
        else:
            self.header = header[0xD0:0x150] + \
                header[0x80:0xD0] + header[:0x80]

    def tag(self):
        return self.header[:0x4]

    def set_tag(self, tag):
        self.header = tag + self.header[0x4:]

    def sha1_hash(self):
        return self.header[0x4:0x18]

    def set_sha1_hash(self, hash):
        self.header = self.header[:0x4] + hash + self.header[0x18:]

    def vanity_area(self):
        return self.header[0x18:0x40]

    def set_vanity_area(self, vanity):
        self.header = self.header[:0x18] + vanity + self.header[0x40:]

    def kirk_block(self):
        return self.header[0x40:0xB0]

    def set_kirk_block(self, block):
        self.header = self.header[:0x40] + block + self.header[0xB0:]

    def kirk_metadata(self):
        return self.header[0xB0:0xD0]

    def set_kirk_metadata(self, metadata):
        self.header = self.header[:0xB0] + metadata + self.header[0xD0:]

    def elf_info(self):
        return self.header[0xD0:]

    def set_elf_info(self, info):
        self.header = self.header[:0xD0] + info

    def kirk_aes_key(self):
        return self.header[0x40:0x50]

    def kirk_ecdsa_header_sig(self):
        return self.header[0x50:0x78]

    def kirk_ecdsa_data_sig(self):
        return self.header[0x78:0xA0]

    def prx(self):
        return self.elf_info() + self.header[0x80:0xD0] + self.header[:0x80]


def decrypt(prx, meta, **kwargs):
    p = prx_header_8(prx)
    xorbuf = kirk.kirk7(meta['seed'], meta['key'])

    # calculate SHA1 of header
    h = SHA1.new()
    h.update(xorbuf[:0x14])
    h.update(p.vanity_area())
    h.update(p.kirk_block())
    h.update(p.kirk_metadata())
    h.update(p.elf_info())

    if h.digest() != p.sha1_hash():
        print("bad SHA1")
        return False

    # decrypt the kirk header
    header = xor(p.kirk_block(), xorbuf[0x14:0x84])
    header = kirk.kirk7(header, meta['key'])
    header = xor(header, xorbuf[0x20:])

    # prepare the kirk block
    block = header + p.kirk_metadata() + p.elf_info() + prx[0x150:]

    # do the decryption
    return kirk.kirk1(block)


def encrypt(prx, meta, vanity=None, **kwargs):
    xorbuf = kirk.kirk7(meta['seed'], meta['key'])

    # encrypt as kirk1
    encrypted = kirk.kirk1_encrypt_ecdsa(prx[0x150:], salt=prx[:0x80])

    header = xor(encrypted[:0x70], xorbuf[0x20:])
    header = kirk.kirk4(header, meta['key'])
    header = xor(header, xorbuf[0x14:0x84])

    # calculate some vanity
    if vanity == None:
        vanity = Random.get_random_bytes(0x28)

    elif type(vanity) is str:
        vanity = '{:40.40}'.format(vanity).encode()

    # create a prx header
    prx_header = prx_header_8()
    prx_header.set_elf_info(prx[:0x80])
    prx_header.set_kirk_block(header)
    prx_header.set_kirk_metadata(encrypted[0x70:0x90])
    prx_header.set_vanity_area(vanity)
    prx_header.set_tag(prx[0xD0:0xD4])

    # calculate SHA1 of header
    h = SHA1.new()
    h.update(xorbuf[:0x14])
    h.update(prx_header.vanity_area())
    h.update(prx_header.kirk_block())
    h.update(prx_header.kirk_metadata())
    h.update(prx_header.elf_info())
    prx_header.set_sha1_hash(h.digest())

    # encrypt the header and return the complete PRX
    return prx_header.prx() + encrypted[0x90+0x80:]
