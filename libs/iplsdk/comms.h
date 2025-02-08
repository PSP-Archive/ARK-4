#pragma once

enum SysconCommand {
    SYSCON_GET_BARYON_VERSION = 1,
    SYSCON_GET_DIGITAL_KEY_KERNEL = 7,
    SYSCON_GET_WAKEUP_FACTOR = 0xE,
    SYSCON_WRITE_SCRATCHPAD = 0x23,
    SYSCON_READ_SCRATCHPAD = 0x24,
    SYSCON_HANDSHAKE_AUTH_UNLOCK = 0x30,
    SYSCON_RESET_DEVICE = 0x32,
    SYSCON_CTRL_HR_POWER = 0x34,
    SYSCON_GET_POMMEL_VERSION = 0x40,
    SYSCON_CTRL_VOLTAGE = 0x42,
    SYSCON_CTRL_POWER = 0x45,
    SYSCON_GET_POWER_STATUS = 0x46,
    SYSCON_CTRL_LED = 0x47,
    SYSCON_CTRL_MS_POWER = 0x4C,
};

int syscon_issue_command_write(enum SysconCommand cmd, const unsigned char *data, unsigned int length);
int syscon_issue_command_read(enum SysconCommand cmd, unsigned char *data);
int syscon_issue_command_read_write(enum SysconCommand cmd, const unsigned char *data, unsigned int length, unsigned char *out_data);
