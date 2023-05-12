import struct
import hashlib
from Crypto.Util.strxor import strxor as xor
from Crypto.Hash import SHA1

import psptool.kirk
import psptool.prxtypes.type2 as type2
import psptool.prxtypes.type6 as type6
import psptool.prxtypes.type8 as type8
import psptool.prxtypes.type9 as type9

_metatypes = {
    0x457B8AF0: {
        'key': 0x5B,
        'seed': '47EC6015122CE3E04A226F319FFA973E',
        'decrypt': type6.decrypt,
        'encrypt': type6.encrypt
    },

    0x457B90F0: {
        'key': 0x5B,
        'seed': 'BA7661478B55A8728915796DD72F780E',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9494F0: {
        'key': 0x43,
        'seed': '76F26C0ACA3ABA4EAC76D240F5C3BFF9',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9495F0: {
        'key': 0x43,
        'seed': '7A3E5575B96AFC4F3EE3DFB36CE82A82',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9496F0: {
        'key': 0x43,
        'seed': 'EBD91E053CAEAB62E3B71F37E5CD68C3',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9490F0: {
        'key': 0x43,
        'seed': 'FA790936E619E8A4A94137188102E9B3',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9484F0: {
        'key': 0x43,
        'seed': '36B0DCFC592A951D802D803FCD30A01B',
        'decrypt': type6.decrypt
    },

    0x4C9485F0: {
        'key': 0x43,
        'seed': '238D3DAE4150A0FAF32F32CEC727CD50',
        'decrypt': type6.decrypt
    },

    0x4C9486F0: {
        'key': 0x43,
        'seed': '8DDBDC5CF2702B40B23D0009617C1060',
        'decrypt': type6.decrypt
    },

    0x457B80F0: {
        'key': 0x5B,
        'seed': 'D43518022968FBA06AA9A5ED78FD2E9D',
        'decrypt': type6.decrypt
    },

    0x457B81F0: {
        'key': 0x5B,
        'seed': 'AAA1B57C935A95BDEF6916FC2B9231DD',
        'decrypt': type6.decrypt
    },

    0x457B82F0: {
        'key': 0x5B,
        'seed': '873721CC65AEAA5F40F66F2A86C7A1C8',
        'decrypt': type6.decrypt
    },

    0x380280F0: {
        'key': 0x5A,
        'seed': '970912D3DB02BDD8E77451FEF0EA6C5C',
        'decrypt': type6.decrypt
    },

    0x457B91F0: {
        'key': 0x5B,
        'seed': 'C59C779C4101E48579C87163A57D4FFB',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x457B92F0: {
        'key': 0x5B,
        'seed': '928CA412D65C55315B94239B62B3DB47',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x380290F0: {
        'key': 0x5A,
        'seed': 'F94A6B96793FEE0A04C88D7E5F383ACF',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x0B000000: {
        'key': 0x4E,
        'seed': '0B011CE731156B833E260DCC693612CBA7FD2666932A6E1A912EC6FCD82F00135AE2DFB6A2E427C818C35050B7E94AEDCC3C30FD106A2B0A22CBC6E0206512EB7D4E2A370B0AEF88DA0654D430AFCDCA9AF9DA1AB01BBB620CDBF8447356148E93B12CFD67E25DCB485BD9B35414D79F799C24E9C27A4E8C4D241994FFC9C22D236351B8FAD67FE65EBC32B20213C476',
        'decrypt': type8.decrypt,
        'encrypt': type8.encrypt
    },

    0x00000000: {
        'key': 0x42,
        'seed': '6A1971F318DED3A26D3BDEC7BE98E24CE3DCDF427B5B12287DC07A5986F0F5B558D8641884247FE957AB4FC6926D7029D3618787D0AE2CE73777C73C967E211F6595C06157AC64D85A6D14D29C54C6685DF5C3F050DAEA1943A7ADC32A14CAC84C838618AE8649FB4F4575D2C3D6E1136937C690CFF979A1773A3EBBBBD53B841B9AB879F0D35F6F4CC02887BCAEDA00',
        'decrypt': type8.decrypt,
        'encrypt': type8.encrypt
    },

    0x0E000000: {
        'key': 0x51,
        'seed': 'DE57B77717DD62EE7B78035D4486CA59208DF69328938121714EA786CA82241B58AE745F6C018D5632884D9A7243A22E84F40C82B906FCFC6AFB5B8AD79C9FBF010D8515BA5FED399383C34CAFDE3AEDBF68A71A778ABD8965415646D9DB3373816CE862969B29035AAEAF732053A040E84B6610996AB7E570DDE029282460EA30AE4220328D6F94715F9EA2D57F0C7C',
        'decrypt': type8.decrypt,
        'encrypt': type8.encrypt
    },

    0x06000000: {
        'key': 0x49,
        'seed': '8415128CA883D780EF1E88DBBC6196232BF388FCE53FB5DE985AA06BDE0A55DBF2F84436EBD194554A393E937C3DE3021288E7F5F8F0C1EB251B8DC6B81E2B44A5B76A7ED03946926D71DE0797B82F1018BADD53C6072B98248A740D5C645DFE8EE767439396B3A1A1A8EC12C4FB5844FC550A9C1E3037FA5424D303E292BD312303EAF7E773DE093BB38379DA17850E',
        'decrypt': type8.decrypt,
        'encrypt': type8.encrypt
    },

    0x02000000: {
        'key': 0x45,
        'seed': '72812FB3395A3DBD388A10749655B1DF889FEEA1B571748956E1A3BB7E9FC3C29EF89BB987BD228857DE1B88C99A3B1ABABDC7A658CB8FA10EDF643B4A9696CB36D04F2D32DD19ABE1D654FE9713575C7A680571347D311E3366DD6D7B76171B259BAF2179177210FDB55535A9BE55AE7245CE55A27080E5ADD0BEB9E47E02A99246C33505F17A93C13A1A48993B3C1B',
        'decrypt': type8.decrypt,
        'encrypt': type8.encrypt
    },

    0xD82310F0: {
        'key': 0x51,
        'seed': '9D09FD20F38F10690DB26F00CCC5512E',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0xD8231EF0: {
        'key': 0x51,
        'seed': '4F445C62B353C430FC3AA45BECFE51EA',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0xD82328F0: {
        'key': 0x51,
        'seed': '5DAA72F226604D1CE72DC8A32F79C554',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0x457B93F0: {
        'key': 0x5B,
        'seed': '88AF18E9C3AA6B56F7C5A8BF1A84E9F3',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9497F0: {
        'key': 0x43,
        'seed': 'BFF834028447BD871C52032379BB5981',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x380293F0: {
        'key': 0x5A,
        'seed': 'CB93123831C02D2E7A185CAC9293AB32',
        'pubkey': '773F4BE14C0AB452672B6756824CCF42AA37FFC08941E5635E84E9FB53DA949E9BB7C2A4229FDF1F',
        'decrypt': type9.decrypt,
        'encrypt': type9.encrypt
    },

    0x4C9414F0: {
        'key': 0x43,
        'seed': '45EF5C5DED81998412948FABE8056D7D',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0x4C9416F0: {
        'key': 0x43,
        'seed': 'EB1B530B624932581F830AF4993D75D0',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0x4C9417F0: {
        'key': 0x43,
        'seed': 'BAE2A31207FF041B64A51185F72F995B',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0x4C941FF0: {
        'key': 0x43,
        'seed': '2C8EAF1DFF79731AAD96AB09EA35598B',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },
    
    0x38020AF0: {
        'key': 0x5A,
        'seed': 'AB8225D7436F6CC195C5F7F063733FE7',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },
    
    0x457B0AF0: {
        'key': 0x5B,
        'seed': 'E8BE2F06B1052AB9181803E3EB647D26',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    },

    0x0C000000: {
        'key': 0x4F,
        'seed': '824CA518D3C86EEA174104DCEAC501FC97B19454711922EEE02DE9833D6430E6425C305FEB41A0E062C663EE5DA50D1EC210144906C6938471A5426313F0B6D543519EFA910A7CE1581B95254011F18DB1018D0409545C54F55308B05385B4CE0BF5C3FBC655240BF2C62CE40CF0053CD76C39D5872209F73DC5A2FD55923FB1F6FEC8181D6B04525F8CE8E7265A6E5A',
        'decrypt': type8.decrypt,
        'encrypt': type8.encrypt
    },
    
    0xADF305F0: {
    	'key': 0x60,
    	'seed': '1299705E24076CD02D06FE7EB30C1126',
        'decrypt': type2.decrypt,
        'encrypt': type2.encrypt
    }
}


def _meta(tag):
    try:
        meta = dict(_metatypes[tag])
        meta['seed'] = bytes.fromhex(meta['seed'])
        return meta
    except KeyError:
        print('missing tag {:08X}'.format(tag))
        raise


def decrypt(prx, **kwargs):
    meta = _meta(struct.unpack('<I', prx[0xD0:0xD4])[0])
    return meta['decrypt'](prx, meta, **kwargs)


def encrypt(prx, tag=None, **kwargs):
    if tag is None:
        tag = struct.unpack('<I', prx[0xD0:0xD4])[0]
    meta = _meta(tag)
    return meta['encrypt'](prx, meta, **kwargs)
