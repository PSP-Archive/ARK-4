#!/usr/bin/env python3

import sys
import struct

from Cryptodome.Cipher import AES
from Cryptodome.Hash import CMAC, SHA1
from Cryptodome import Random
from ecdsa.ellipticcurve import CurveFp, Point
from ecdsa.curves import Curve
from ecdsa import SigningKey

_k4_k7_keys = [
    '2C92E5902B86C106B72EEA6CD4EC7248',
    '058DC80B33A5BF9D5698FAE0D3715E1F',
    'B813C35EC64441E3DC3C16F5B45E6484',
    '9802C4E6EC9E9E2FFC634CE42FBB4668',
    '99244CD258F51BCBB0619CA73830075F',
    '0225D7BA63ECB94A9D237601B3F6AC17',
    '6099F28170560E5F747CB520C0CDC23C',
    '76368B438F77D87EFE5FB6115939885C',
    '14A115EB434A1BA4905E03B617A15C04',
    'E65803D9A71AA87F059D229DAF5453D0',
    'BA3480B428A7CA5F216412F70FBB7323',
    '72AD35AC9AC3130A778CB19D88550B0C',
    '8485C848750843BC9B9AECA79C7F6018',
    'B5B16EDE23A97B0EA17CDBA2DCDEC46E',
    'C871FDB3BCC5D2F2E2D7729DDF826882',
    '0ABB336C96D4CDD8CB5F4BE0BADB9E03',
    '32295BD5EAF7A34216C88E48FF50D371',
    '46F25E8E4D2AA540730BC46E47EE6F0A',
    '5DC71139D01938BC027FDDDCB0837D9D',
    '51DD65F071A4E5EA6AAF12194129B8F4',
    '03763C6865C69B0FFE8FD8EEA43616A0',
    '7D50B85CAF6769F0E54AA8098B0EBE1C',
    '72684B32AC3B332F2A7AFC9E14D56F6B',
    '201D31964AD99FBF32D5D61C491BD9FC',
    'F8D84463D610D12A448E9690A6BB0BAD',
    '5CD4057FA13060440AD9B6745F244F4E',
    'F48AD678599C22C1D411933DF845B893',
    'CAE7D287A2ECC1CD94542B5E1D9488B2',
    'DE26D37A39956C2AD8C3A6AF21EBB301',
    '7CB68B4DA38D1DD932679CA99FFB2852',
    'A0B556B469AB368F36DEC9092ECB41B1',
    '939DE19B725FEEE2452ABC1706D14769',
    'A4A4E621382EF1AF7B177AE842AD0031',
    'C37F13E8CF84DB34747BC3A0F19D3A73',
    '2BF7838AD898E95FA5F901DA61FE35BB',
    'C704621E714A66EA62E04B203DB8C2E5',
    'C933859AAB00CDCE4D8B8E9F3DE6C00F',
    '1842561F2B5F34E3513EB78977431A65',
    'DCB0A0065A50A14E59AC973F1758A3A3',
    'C4DBAE83E29CF254A3DD374E807BF425',
    'BFAEEB498265C57C64B8C17E19064409',
    '797CECC3B3EE0AC03BD8E6C1E0A8B1A4',
    '7534FE0BD6D0C28D68D4E02AE7D5D155',
    'FAB35326974F4EDFE4C3A814C32F0F88',
    'EC97B386B433C6BF4E539D95EBB979E4',
    'B320A204CF480629B5DD8EFC98D4177B',
    '5DFC0D4F2C39DA684A3374ED4958A73A',
    'D75A5422CED9A3D62B557D8DE8BEC7EC',
    '6B4AEE4345AE7007CF8DCF4E4AE93CFA',
    '2B522F664C2D114CFE61318C56784EA6',
    '3AA34E44C66FAF7BFAE55327EFCFCC24',
    '2B5C78BFC38E499D41C33C5C7B2796CE',
    'F37EEAD2C0C8231DA99BFA495DB7081B',
    '708D4E6FD1F66F1D1E1FCB02F9B39926',
    '0F6716E180699C51FCC7AD6E4FB846C9',
    '560A494A844C8ED982EE0B6DC57D208D',
    '12468D7E1C42209BBA5426835EB03303',
    'C43BB6D653EE67493EA95FBC0CED6F8A',
    '2CC3CF8C2878A5A663E2AF2D715E86BA',
    '833DA70CED6A2012D196E6FE5C4D37C5',
    'C743D06742EE90B8CA75503520ADBCCE',
    '8AE3663F8D9E82A1EDE68C9CE8256DAA',
    '7FC96F0BB1485CA55DD364B77AF5E4EA',
    '91B765788BCB8BD402ED553A6662D0AD',
    '2824F9101B8D0F7B6EB263B5B55B2EBB',
    '30E2575DE0A249CEE8CF2B5E4D9F52C7',
    '5EE50439623202FA85393F72BB77FD1A',
    'F88174B1BDE9BFDD45E2F55589CF46AB',
    '7DF49265E3FAD678D6FE78ADBB3DFB63',
    '747FD62DC7A1CA96E27ACEFFAA723FF7',
    '1E58EBD065BBF168C5BDF746BA7BE100',
    '24347DAF5E4B35727A52276BA05474DB',
    '09B1C705C35F536677C0EB3677DF8307',
    'CCBE615C05A20033378E5964A7DD703D',
    '0D4750BBFCB0028130E184DEA8D48413',
    '0CFD679AF9B4724FD78DD6E99642288B',
    '7AD31A8B4BEFC2C2B39901A9FE76B987',
    'BE787817C7F16F1AE0EF3BDE4CC2D786',
    '7CD8B891910A4314D0533DD84C45BE16',
    '32722C8807CF357D4A2F511944AE68DA',
    '7E6BBFF6F687B898EEB51B3216E46E5D',
    '08EA5A8349B59DB53E0779B19A59A354',
    'F31281BFE69F51D164082521FFBB2261',
    'AFFE8EB13DD17ED80A61241C959256B6',
    '92CDB4C25BF2355A2309E819C9144235',
    'E1C65B226BE1DA02BA18FA21349EF96D',
    '14EC76CE97F38A0A34506C539A5C9AB4',
    '1C9BC490E3066481FA59FDB600BB2870',
    '43A5CACC0D6C2D3F2BD989676B3F7F57',
    '00EFFD1808A405893C38FB2572706106',
    'EEAF49E009879BEFAAD6326A3213C429',
    '8D26B90F431DBB08DB1DDAC5B52C92ED',
    '577C3060AE6EBEAE3AAB1819C571680B',
    '115A5D20D53A8DD39CC5AF410F0F186F',
    '0D4D51AB2379BF803ABFB90E75FC14BF',
    '9993DA3E7D2E5B15F252A4E66BB85A98',
    'F42830A5FB0D8D760EA671C22BDE669D',
    'FB5FEB7FC7DCDD693701979B29035C47',
    '02326AE7D396CE7F1C419DD65207ED09',
    '9C9B1372F8C640CF1C62F5D592DDB582',
    '03B302E85FF381B13B8DAA2A90FF5E61',
    'BCD7F9D32FACF847C0FB4D2F309ABDA6',
    'F55596E97FAF867FACB33AE69C8B6F93',
    'EE297093F94E445944171F8E86E170FC',
    'E434520CF088CFC8CD781B6CCF8C48C4',
    'C1BF66818EF953F2E1266B6F550CC9CD',
    '560FFF8F3C9649144516F1BCBFCEA30C',
    '2408DC753760A29F0554B5F243857399',
    'DDD5B56A59C55AE83B9667C75C2AE2DC',
    'AA686772E02D44D5CDBB6504BCD5BF4E',
    '1F17F014E777A2FE4B136B56CD7EF7E9',
    'C93548CF558D7503896B2EEB618CA902',
    'DE34C541E7CA86E8BEA7C31CECE4360F',
    'DDE5FF551B74F6F4E016D7AB22311B6A',
    'B0E93521333FD7BAB4762CCB4D8008D8',
    '381469C4C3F91B9633638E4D5F3DF029',
    'FA486AD98E6716EF6AB087F589457F2A',
    '321A091250148A3E963DEA025932E18F',
    '4B00BE29BCB02864CEFD43A96FD95CED',
    '577DC4FF0244E28091F4CA0A7569FDA8',
    '835336C61803E43E4EB30F6B6E799B7A',
    '5C9265FD7B596AA37A2F509D85E927F8',
    '9A39FB89DF55B2601424CEA6D9650A9D',
    '8B75BE91A8C75AD2D7A594A01CBB9591',
    '95C21B8D05ACF5EC5AEE77812395C4D7',
    'B9A461643633FA5D9488E2D3281E01A2',
    'B8B084FB9F4CFAF730FE7325A2AB897D',
    '5F8C179FC1B21DF1F6367A9CF7D3D47C'
]

def kirk4(data, key):
    aes = AES.new(bytes.fromhex(_k4_k7_keys[key]), AES.MODE_CBC, iv=b'\x00'*16)
    return aes.encrypt(data)

def kirk7(data, key):
    aes = AES.new(bytes.fromhex(_k4_k7_keys[key]), AES.MODE_CBC, iv=b'\x00'*16)
    return aes.decrypt(data)

class Kirk1(object):
    def __init__(self):
        self.key = bytes.fromhex('98C940975C1D10E87FE60EA3FD03A8BA')
        self.iv = bytes.fromhex('00000000000000000000000000000000')

        p = 0xFFFFFFFFFFFFFFFF00000001FFFFFFFFFFFFFFFF
        a = -3
        b = 0x65D1488C0359E234ADC95BD3908014BD91A525F9
        Gx = 0x2259ACEE15489CB096A882F0AE1CF9FD8EE5F8FA
        Gy = 0x604358456D0A1CB2908DE90F27D75C82BEC108C0
        r = 0xffffffffffffffff0001b5c617f290eae1dbad8f

        curve = CurveFp(p, a, b)
        generator = Point(curve, Gx, Gy, r)
        self.curve = Curve("KIRK", curve, generator, (1, 3, 3, 7, 4))
        self.sk = SigningKey.from_string(bytes.fromhex('F392E26490B80FD889F2D9722C1F34D7274F983D'), curve=self.curve)
        self.vk = self.sk.get_verifying_key()
        self.k1_enc = AES.new(self.key, AES.MODE_CBC, self.iv)
        self.k1_dec = AES.new(self.key, AES.MODE_CBC, self.iv)

    @classmethod
    def header_size(clazz):
        return 0x60 + struct.calcsize('<II8xII8x16x')
    
    @classmethod
    def vanity_keys(clazz, aes_k, cmac_k=None):
        key = bytes.fromhex('98C940975C1D10E87FE60EA3FD03A8BA')
        iv = bytes.fromhex('00000000000000000000000000000000')
        k1_enc = AES.new(key, AES.MODE_CBC, iv)

        if cmac_k:
            key_bundle = aes_k + cmac_k
        else:
            key_bundle = aes_k
        
        dec = k1_enc.decrypt(key_bundle)

        if cmac_k:
            return dec[:0x10], dec[0x10:]
        else:
            return dec[:0x10], None

    def decrypt(self, block):
        mode = struct.unpack("<I", block[0x60:0x64])[0]
        ecdsa = struct.unpack("<I", block[0x64:0x68])[0]
        size = struct.unpack("<I", block[0x70:0x74])[0]
        offset = struct.unpack("<I", block[0x74:0x78])[0]
        padding = 0

        while (size + offset + padding) % 16 != 0:
            padding = padding + 1

        size = size + padding

        # decrypt our kirk keys
        keys = self.k1_dec.decrypt(block[0x00:0x20])

        if ecdsa == 1:
            self.__verify_ecdsa(block, size, offset)
        else:
            self.__verify_cmac(keys[0x10:0x20], block, size, offset)

        aes = AES.new(keys[0x00:0x10], AES.MODE_CBC, self.iv)
        return aes.decrypt(block[0x90+offset:0x90+offset+size])

    def encrypt(self, data, salt=b'', key=None, ecdsa=False, cmac_key=None):
        if key == None:
            key = Random.get_random_bytes(0x10)

        return self.__encrypt_ecdsa(data, key, salt) if ecdsa else self.__encrypt_cmac(data, key, cmac_key, salt)

    def __encrypt_ecdsa(self, data, key, salt):
        aes = AES.new(key, AES.MODE_CBC, self.iv)

        # generate header and sign
        kirk_header2 = struct.pack('<II8xII8x16x', 1, 1, len(data), len(salt))
        header_signature = self.sk.sign(kirk_header2)

        # generate second part of block and sign
        block = kirk_header2 + salt + aes.encrypt(data)
        data_signature = self.sk.sign(block)

        # encrypt the aes key and package signatures
        block = self.k1_enc.encrypt(key) + header_signature + data_signature + block

        # verify our signatures are valid
        self.__verify_ecdsa(block, len(data), len(salt))
        return block

    def __encrypt_cmac(self, data, key, cmac_key, salt):
        aes = AES.new(key, AES.MODE_CBC, self.iv)
        cmac_key = cmac_key if cmac_key != None else Random.get_random_bytes(0x10)
        cmac = CMAC.new(cmac_key, ciphermod=AES)

        # generate header and mac
        kirk_header2 = struct.pack('<II8xII8x16x', 1, 0, len(data), len(salt))
        cmac.update(kirk_header2)
        header_mac = cmac.digest()

        # generate second part of block and mac
        block = kirk_header2 + salt + aes.encrypt(data)
        cmac = CMAC.new(cmac_key, ciphermod=AES)
        cmac.update(block)
        data_mac = cmac.digest()

        # encrypt the aes key and package mac
        block = self.k1_enc.encrypt(key + cmac_key) + header_mac + data_mac + b'\x00'*0x20 + block

        # verify our signatures are valid
        self.__verify_cmac(cmac_key, block, len(data), len(salt))
        return block

    def __verify_ecdsa(self, block, size, offset):
        self.vk.verify(block[0x10:0x10+0x28], block[0x60:0x60+0x30])
        self.vk.verify(block[0x38:0x38+0x28], block[0x60:0x60+0x30+size+offset])

    def __verify_cmac(self, key, block, size, offset):
        cobj = CMAC.new(key, ciphermod=AES)
        cobj.update(block[0x60:0x60+0x30])
        cobj.verify(block[0x20:0x30])

        cobj = CMAC.new(key, ciphermod=AES)
        cobj.update(block[0x60:0x90+size+offset])
        cobj.verify(block[0x30:0x40])

# an IPL is MAX 0x1000 bytes per block, including any necessary headers
# IPLs require a minimum of 0x90 bytes for the KIRK1 header, and 0x20 bytes
# for the encrypted hash. All blocks must include 0x10 bytes IPL header and
# Final blocks have the additional restriction of holding an ECDSA signature.
# we cannot forge these signatures, but one day if we can, we must reserve
# another 0x40 bytes for it
#
# so normal IPL block can contain 0x1000 - 0x90 - 0x20 - 0x10 = 0xF40 image bytes
# and a final block IPL can contain 0xF00 image bytes
MAX_IMG_BLOCK_SIZE = 0xF40
MAX_FINAL_IMG_BLOCK_SIZE = 0xF00

# the bootrom will only work on blocks that are a MINIMUM of 0x100 bytes
MIN_KIRK_SIZE = 0x100

def calc_checksum(image):
    # calculate the checksum. its a simple accumulator
    checksum = 0
    for i in range(0, len(image), 4):
        checksum += struct.unpack('<I', image[i:i+4])[0]
        checksum &= 0xFFFFFFFF
    return checksum

def encrypt_ipl_block(image_segment, load_address, entry, checksum, vanity=b'', padding = b''):
    # build a header for the image. only insert entry if its the last block
    header = struct.pack('<IIII', load_address, len(image_segment), entry, checksum)
    dec_block = header + image_segment

    # calculate the SHA1
    block_hash = SHA1.new(dec_block[8:] + dec_block[:8]).digest() + b'\x0C'*12
    print(f'got block hash: {block_hash.hex()}')
    assert len(block_hash) == 0x20

    # block size should be aligned to 16 bytes already
    assert (len(dec_block) % 16) == 0

    # calculate the checksum. its a simple accumulator
    checksum = calc_checksum(image_segment)

    # now we can perform the encryption
    kirk1 = Kirk1()
    block = kirk1.encrypt(dec_block + padding, salt=vanity, ecdsa=True)

    # if we have a full block we expect 0xFE0 bytes
    if not full_block:
        assert(len(block) != 0xFE0)
        block += b'\x00'*(0xFE0-len(block))
    else:
        assert(len(block) == 0xFE0)

    # encrypt SHA1 and append it to the image
    block = block + kirk4(block_hash, 0x6C)
    assert(len(block) == 0x1000)

    return (block, checksum)

def reset_ipl_block(load_address, checksum, image=None):
    # this assumes that checksum is zero. it needs to be controlled
    # since it will technically be executed as a MIPS instruction
    assert checksum == 0

    # generate the IPL header. it is designed to write 0x32F6 to 0xBC10004C to force a reset
    # these instructions will be execute, so there is a branch to +0x20 to avoid any
    # strange behaviour
    ipl_hdr = struct.pack('<IIII', 0xBC10004C, 4, 0x10000005, checksum)
    exploit_addition = struct.pack('<IIII', 0x02F6, 0, 0, 0)

    if not image:
        # the code ran. our payload is at 'load_address'. we are simply going to jump to it
        # 
        # lui   $t9, load_address >> 16
        # ori   $t9, $t9, load_address & 0xFFFF
        # jr    $t9
        # li    $v0, 0xDAEE
        #
        # $t9 is set to the load_addr and $v0 is also set to 0xDAEE so the next stage
        # can determine if we booted from bootrom or not. if there is a better way then i'd
        # like to hear it
        lui_t9_upper = 0x3C190000
        ori_t9_lower = 0x37390000
        jr_t9 = 0x03200008
        li_v0_daee = 0x3402DAEE

        code = struct.pack('<IIII', lui_t9_upper | ((load_address >> 16) & 0xFFFF), ori_t9_lower | (load_address & 0xFFFF), jr_t9, li_v0_daee)
    else:
        code = image

    # package it all up into a block
    dec_block = ipl_hdr + exploit_addition + code

    # calculate SHA1 hash
    block_hash = SHA1.new(dec_block[8:0x10+4] + dec_block[:8]).digest() + b'\x0C'*12
    assert len(block_hash) == 0x20

    # block size should be aligned to 16 bytes already
    assert (len(dec_block) % 16) == 0

    reset_padding = (b'RESET '*0x1000)

    aesk, cmack = Kirk1.vanity_keys(reset_padding[:0x10], reset_padding[0x10:0x20])
    reset_padding = reset_padding[0x90:]

    vanity_salt = reset_padding[:0x30]
    reset_padding = reset_padding[0x60:]

    # we need to meet the minimum size for a kirk block
    if len(dec_block) < MIN_KIRK_SIZE:
        pad_num = (MIN_KIRK_SIZE - len(dec_block))
        padding = reset_padding[:pad_num]
        reset_padding = reset_padding[pad_num:]

    # this is just vanity
    aes = AES.new(aesk, mode=AES.MODE_CBC, iv=b'\x00'*16)
    last_iv = aes.encrypt(dec_block)[-0x10:]
    aes = AES.new(aesk, mode=AES.MODE_CBC, iv=last_iv)
    padding = aes.decrypt(padding)
    
    kirk1 = Kirk1()
    block = kirk1.encrypt(dec_block + padding, key=aesk, cmac_key=cmack, salt=vanity_salt, ecdsa=False)

    if (len(block) != 0xFE0):
        block += reset_padding[:(0xFE0-len(block))]

    assert len(block) == 0xFE0
    return block + kirk4(block_hash, 0x6C)

def print_usage():
    print('usage: {} [binary] [ipl out] [type (normal/reset_block)] [load address] (vanity optional)'.format(sys.argv[0]))

if len(sys.argv) != 5 and len(sys.argv) != 6:
    print_usage()
    sys.exit(1)

data = open(sys.argv[1], 'rb').read()
out = open(sys.argv[2], 'wb')

ipl_type = sys.argv[3]
is_reset_block = False
is_reset_block_app = False
is_reset_block_build = False
load_address = int(sys.argv[4], 16) #0x40c0000

if ipl_type == "normal":
    pass
elif ipl_type == "reset_block":
    is_reset_block = True
elif ipl_type == "reset_block_app":
    is_reset_block_app = True
    is_reset_block = True
elif ipl_type == "reset_block_app_bld":
    is_reset_block_build = True
else:
    print('ipl type can be "normal" or "reset_block"')
    sys.exit(1)

if is_reset_block_build:
    # first we pad the image to a multiple of 0x10
    if (len(data) % 0x10) != 0:
        image = data + b'\x00'*(0x10 - (len(data) % 0x10))
    else:
        image = data

    if len(data) > MAX_IMG_BLOCK_SIZE:
        print("too big")
        sys.exit(1)

    out.write(reset_ipl_block(load_address, 0, image=image))
    sys.exit(0)

# if its a reset block add 4 bytes for checksum
if is_reset_block:
    data = data + b'\x00'*4

# first we pad the image to a multiple of 0x10
if (len(data) % 0x10) != 0:
    image = data + b'\x00'*(0x10 - (len(data) % 0x10))
else:
    image = data

image_size = len(image)

# check if there is a vanity
if len(sys.argv) == 6 and not is_reset_block_app:
    vanity = sys.argv[5]
else:
    vanity = b''

if vanity != 'I_DEMAND_NO_ADS':
    # this is free software, so i'm taking 64 bytes to stamp some credits into
    # your final image. you can figure out what needs to be done if you really
    # don't want this
    #literally_ads = b'Ma tha thusa na d\' fhear-ealaidh, cluinneamaid annasdo laimhe.\x00\x00'
    literal_response = b'...et audistis me' + b'\x00'*47
    assert(len(literal_response) == 64)

    if len(vanity) % 16 != 0:
        vanity = vanity + b'\x00'*(0x10 - (len(vanity) % 16))
    
    vanity = vanity + literal_response
else:
    vanity = b''

ipl_img = b''
checksum = 0
offset = 0
has_final = False

while len(image) > 0:
    padding = b''

    # consume as many bytes as we can
    if len(image) >= MAX_IMG_BLOCK_SIZE - len(vanity):
        image_segment = image[:MAX_IMG_BLOCK_SIZE - len(vanity)]
        full_block = True
    else:
        if len(image) < MIN_KIRK_SIZE:
            padding = b'\x00'*(MIN_KIRK_SIZE - len(image))
            image_segment = image
        else:
            image_segment = image
        full_block = False
    
    # if we can fit in a final block lets do that here
    if len(image) < MAX_FINAL_IMG_BLOCK_SIZE - len(vanity):
        if not is_reset_block:
            has_final = True

    print(f'full: {full_block}, len(image): {len(image)}, len(seg): {len(image_segment)}, is_reset: {is_reset_block}')
    # if its a reset block we need to add a checksum for adjustment
    # for the last block
    if len(image) == len(image_segment) and is_reset_block:
        # calculate the current checksum minus 4 bytes for an adjustment
        next_csum = calc_checksum(image_segment[:-4])

        # calculate the value required to manipulate it to a zero
        checksum_mod = (0 - next_csum) & 0xFFFFFFFF
        image_segment = image_segment[:-4] + struct.pack('<I', checksum_mod)

        # do a sanity check
        next_csum = calc_checksum(image_segment)
        assert next_csum == 0

    # if we have the final block here then we mark the entry
    if has_final:
        entry = load_address
    else:
        entry = 0

    # if we're dealing with a reset block then we need to force the bootrom to write
    # without the cache. cache contents will be discarded on reset
    if is_reset_block:
        # encrypt the block and append it to the IPL image
        block, checksum = encrypt_ipl_block(image_segment, 0x4000_0000 | (load_address + offset), entry, checksum, vanity=vanity, padding=padding)
    else:
        # encrypt the block and append it to the IPL image
        block, checksum = encrypt_ipl_block(image_segment, load_address + offset, entry, checksum, vanity=vanity, padding=padding)
    ipl_img = ipl_img + block

    vanity = b''
    offset += len(image_segment)
    image = image[len(image_segment):]

# all the data is encrypted and packed into IPL. however, there is an
# edge case where there isnt enough data to fill a normal block, but 
# too much to fill a final block. we handle that here by encrypting
# an empty block
if not has_final:
    # an exploit exists where we can skip the ECDSA signature completely
    # by crafting a special payload that writes directly to the reset register.
    # an attacker can carefully construct the IPL block to control the data
    # in 0xBFD00000 (where the bootrom performs block decryption). Upon
    # reset this region is re-mapped to 0xBFC00000 (the exception vector). As
    # a result, once the CPU comes out of reset it immediately starts executing
    # the contents that was written to 0xBFD00000 prior to reset.
    if is_reset_block:
        if is_reset_block_app:
            reset_blk = open(sys.argv[5], 'rb').read()
        else:
            reset_blk = reset_ipl_block(load_address, checksum)
        ipl_img = ipl_img + reset_blk
    else:
        # otherwise we just have a normal block
        block, _ = encrypt_ipl_block(b'\x00'*MIN_KIRK_SIZE, 0, load_address, checksum)
        ipl_img = ipl_img + block

out.write(ipl_img)
