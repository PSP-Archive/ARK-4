#ifndef REBOOTBIN_H
#define REBOOTBIN_H

#define REBOOT150_TEXT (KERNEL_BASE + 0xc00000)
#define REBOOT150_SIZE 0x16d40
#define REBOOT_HEADER_SIZE 0x10
#define REBOOT150_HEADER 'r',  'e',  'b',  'o',  'o',  't',  '.',  'b',  'i',  'n',  0x00, 0x00, 0xFB, 0xC5, 0x00, 0x00
#define REBOOT661_HEADER 'r',  'e',  'b',  'o',  'o',  't',  '.',  'b',  'i',  'n',  0x00, 0x00, 0x08, 0x2E, 0x01, 0x00

#endif