#include <pspkernel.h>
#include <pspidstorage.h>
#include <pspsysmem_kernel.h>

#include <stdio.h>
#include <string.h>

#include "systemctrl.h"
#include "macros.h"

// Credits to @Moment

static unsigned char wpa2_seed[] = { 
    0x5c, 0xf7, 0x15, 0x51, 0xa2, 0x56, 0x46, 0x30,
    0x25, 0xe2, 0x80, 0x43, 0x8a, 0xf5, 0x74, 0xdc,
    0xd8, 0xaf, 0xe6, 0x2e, 0xca, 0xab, 0x52, 0x8f,
    0x8a, 0x11, 0x93, 0x07, 0x74, 0x54, 0x12, 0xf7
};

static const unsigned char rsn_info[] = { 
    0x01, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x01, 0x00, 
    0x00, 0x0f, 0xac, 0x04, 0x01, 0x00, 0x00, 0x0f, 
    0xac, 0x02, 0x00, 0x00 
};

// We're only interested in the WiFi network module (sceNetApctl_Library)
void patchSceNetWpa2(SceModule2 *mod)
{
    
    // Patch return value (li v0, 4). Then WPA2 will be recognised as WPA!
    _sw(LI_V0(4), mod->text_addr + 0x14dc);

    // Patch header length (0x75)
    _sw(0x24050075, mod->text_addr + 0xef88);

    // Set EAPOL key type to 2 (WPA2 handshake)
    _sw(0x24190002, mod->text_addr + 0x0000ef98);

    // Set EAPOL key data length to 0x16
    _sw(0x24080016, mod->text_addr + 0xf038);

    // Define RSN tag (48)
    _sw(0x24060030, mod->text_addr + 0xf054);

    // Define RSN tag length (20)
    _sw(0x24050014, mod->text_addr + 0xf058);

    // Fill in RSN information
    memcpy((char *)mod->text_addr + 0x11DA8, rsn_info, sizeof(rsn_info));

    // Ensure RSN info gets copied properly
    _sw(0x24060014, mod->text_addr + 0xf06c);

    // Replace WPA2 seed
    memcpy((char *)mod->text_addr + 0x11880, wpa2_seed, sizeof(wpa2_seed));

    // Kill stores over our EAPOL 2 key to prevent issues
    _sw(0, mod->text_addr + 0xf07c);
    _sw(0, mod->text_addr + 0xf080);
    _sw(0, mod->text_addr + 0xf0a0);
    _sw(0, mod->text_addr + 0xf18c);
    _sw(0, mod->text_addr + 0xf0b0);
    _sw(0, mod->text_addr + 0xf0bc);
    _sw(0, mod->text_addr + 0xf0c0);
    _sw(0, mod->text_addr + 0xf0c8);
    _sw(0, mod->text_addr + 0xf0cc);
    _sw(0, mod->text_addr + 0xf168);

    // Fix MIC size (should match WPA2 standard)
    _sb(0x79, mod->text_addr + 0xf104);
    _sb(0x75, mod->text_addr + 0xf134);

    // Change EAPOL type to RSN Key (ensures WPA2 handshake)
    _sb(0x02, mod->text_addr + 0xf1e8);
    _sb(0x02, mod->text_addr + 0xf680);
    _sb(0x02, mod->text_addr + 0xf518);
    _sb(0x02, mod->text_addr + 0xf344);
    _sb(0x02, mod->text_addr + 0xe0d0);

    // Set security flag
    _sh(0x308, mod->text_addr + 0xf220);

    // WPA2 doesn't use EAPOL start, so disable it
    _sw(0, mod->text_addr + 0xe9f8);

    // APs using WPA2 won't send a GTK update immediately, so skip waiting for it
    uintptr_t end_msg_4 = mod->text_addr + 0xf464;
    _sw(0x3C050000 | (end_msg_4 >> 16), mod->text_addr + 0xf2e0);
    _sw(0x34A50000 | (end_msg_4 & 0xFFFF), mod->text_addr + 0xf2e4);
    _sw(0x00A00008, mod->text_addr + 0xf2e8);
    _sw(0x02009825, mod->text_addr + 0xf2ec);
}
