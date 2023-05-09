import psptool.kirk as kirk


class prx_header(object):
    def __init__(self, header):
        self.header = header

    def elf_info(self):
        return self.header[:0x80]

    def kirk_aes_key(self):
        return self.header[0x80:0x90]

    def kirk_cmac_key(self):
        return self.header[0x90:0xA0]

    def kirk_cmac_header_hash(self):
        return self.header[0xA0:0xB0]

    def kirk_metadata(self):
        return self.header[0xB0:0xC0]

    def kirk_cmac_data_hash(self):
        return self.header[0xC0:0xD0]

    def personalisation(self):
        return self.header[0xD0:0x12C]

    def sha1_hash(self):
        return self.header[0x12C:0x140]

    def btcnf_id(self):
        return self.header[0x140:0x150]


def set_kirk_cmd_1(block):
    return block[:0x60] + (1).to_bytes(4, 'little') + block[0x64:]


def set_kirk_cmd_1_ecdsa(block):
    return block[:0x64] + (1).to_bytes(4, 'little') + block[0x68:]


def expand_seed(seed, key):
    return kirk.kirk7(b''.join([bytes([x])+seed[1:] for x in range(9)]), key)
