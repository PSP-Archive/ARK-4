import struct
import hashlib
from Crypto.Cipher import AES
from Crypto.Hash import CMAC
from ecdsa.ellipticcurve import CurveFp, Point
from ecdsa.curves import Curve
from ecdsa import SigningKey, VerifyingKey
from ecdsa.numbertheory import inverse_mod

_k1_key = '98C940975C1D10E87FE60EA3FD03A8BA'

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


def _kirk1_curve():
    # ECDSA curve
    p = 0xFFFFFFFFFFFFFFFF00000001FFFFFFFFFFFFFFFF
    a = -3
    b = 0x65D1488C0359E234ADC95BD3908014BD91A525F9
    Gx = 0x2259ACEE15489CB096A882F0AE1CF9FD8EE5F8FA
    Gy = 0x604358456D0A1CB2908DE90F27D75C82BEC108C0
    r = 0xffffffffffffffff0001b5c617f290eae1dbad8f
    curve = CurveFp(p, a, b)
    generator = Point(curve, Gx, Gy, r)
    return Curve("kirk1", curve, generator, (1, 3, 3, 7, 4))


def _kirk11_curve():
    # ECDSA curve
    p = 0xFFFFFFFFFFFFFFFF00000001FFFFFFFFFFFFFFFF
    a = -3
    b = 0xA68BEDC33418029C1D3CE33B9A321FCCBB9E0F0B
    Gx = 0x128EC4256487FD8FDF64E2437BC0A1F6D5AFDE2C
    Gy = 0x5958557EB1DB001260425524DBC379D5AC5F4ADF
    r = 0xFFFFFFFFFFFFFFFEFFFFB5AE3C523E63944F2127
    curve = CurveFp(p, a, b)
    generator = Point(curve, Gx, Gy, r)
    return Curve("kirk11", curve, generator, (1, 3, 3, 7, 4))


_k_ecdsa_sign = SigningKey.from_string(bytes.fromhex(
    'F392E26490B80FD889F2D9722C1F34D7274F983D'), curve=_kirk1_curve())
_k_ecdsa_verify = _k_ecdsa_sign.get_verifying_key()


def _verify_cmac(key, block, size, offset):
    cobj = CMAC.new(key, ciphermod=AES)
    cobj.update(block[0x60:0x60+0x30])
    cobj.verify(block[0x20:0x30])
    cobj = CMAC.new(key, ciphermod=AES)
    cobj.update(block[0x60:0x90+size+offset])
    cobj.verify(block[0x30:0x40])


def _verify_ecdsa(block, size, offset):
    _k_ecdsa_verify.verify(block[0x10:0x10+0x28], block[0x60:0x60+0x30])
    _k_ecdsa_verify.verify(block[0x38:0x38+0x28],
                           block[0x60:0x60+0x30+size+offset])


def kirk1(block):
    mode = struct.unpack("<I", block[0x60:0x64])[0]
    ecdsa = struct.unpack("<I", block[0x64:0x68])[0]
    size = struct.unpack("<I", block[0x70:0x74])[0]
    offset = struct.unpack("<I", block[0x74:0x78])[0]
    padding = 0

    while (size + offset + padding) % 16 != 0:
        padding = padding + 1

    size = size + padding

    # decrypt our kirk keys
    k1_aes = AES.new(bytes.fromhex(_k1_key), AES.MODE_CBC, iv=b'\x00'*16)
    keys = k1_aes.decrypt(block[0x00:0x20])

    if ecdsa == 1:
        _verify_ecdsa(block, size, offset)
    else:
        _verify_cmac(keys[0x10:0x20], block, size, offset)

    aes = AES.new(keys[0x00:0x10], AES.MODE_CBC, iv=b'\x00'*16)
    return aes.decrypt(block[0x90+offset:0x90+offset+size])[:size-padding]


def kirk1_encrypt_ecdsa(data, salt=b'', key=None):
    if key is None:
        key = bytearray.fromhex('AA'*16)

    # pad to 16 byte boundary if required
    padding = b''
    if len(data) % 16:
    	for i in range(15, len(data) % 16-1, -1):
    		padding += bytes([i << 4 | i])

    # encrypt the data
    aes = AES.new(key, AES.MODE_CBC, iv=b'\x00'*16)
    enc_data = aes.encrypt(data + padding)

    # sign the header
    header = struct.pack('<II8xII8x16x', 1, 1, len(data), len(salt))
    header_sig = _k_ecdsa_sign.sign(header)

    # sign the data
    block = header + salt + enc_data
    data_sig = _k_ecdsa_sign.sign(block)

    # encrypt keys and return result
    k1_aes = AES.new(bytes.fromhex(_k1_key), AES.MODE_CBC, iv=b'\x00'*16)
    return k1_aes.encrypt(key) + header_sig + data_sig + block


def kirk1_encrypt_cmac(data, salt=b'', aes_key=None, cmac_key=None):
    if aes_key is None:
        aes_key = bytearray.fromhex('AA'*16)

    if cmac_key is None:
        cmac_key = bytearray.fromhex('AA'*16)

    # pad to 16 byte boundary if required
    padding = b''
    if len(data) % 16:
    	for i in range(15, len(data) % 16-1, -1):
    		padding += bytes([i << 4 | i])

    # encrypt the data
    aes = AES.new(aes_key, AES.MODE_CBC, iv=b'\x00'*16)
    enc_data = aes.encrypt(data + padding)

    # cmac the header
    header = struct.pack('<II8xII8x16x', 1, 0, len(data), len(salt))
    cmac = CMAC.new(cmac_key, ciphermod=AES)
    cmac.update(header)
    header_mac = cmac.digest()

    # cmac the data
    block = header + salt + enc_data
    cmac = CMAC.new(cmac_key, ciphermod=AES)
    cmac.update(block)
    data_mac = cmac.digest()

    # encrypt keys and return result
    k1_aes = AES.new(bytes.fromhex(_k1_key), AES.MODE_CBC, iv=b'\x00'*16)
    return k1_aes.encrypt(aes_key + cmac_key) + header_mac + data_mac + b'\x00'*0x20 + block


def kirk4(data, key):
    aes = AES.new(bytes.fromhex(_k4_k7_keys[key]), AES.MODE_CBC, iv=b'\x00'*16)
    return aes.encrypt(data)


def kirk7(data, key):
    aes = AES.new(bytes.fromhex(_k4_k7_keys[key]), AES.MODE_CBC, iv=b'\x00'*16)
    return aes.decrypt(data)


def kirk11(pubkey, sig, data):
    vk = VerifyingKey.from_string(pubkey, curve=_kirk11_curve())
    vk.verify(sig, data)
