#pragma once

enum SysconCommand {
    SYSCON_GET_BARYON_VERSION = 1,
    SYSCON_HANDSHAKE_AUTH_UNLOCK = 0x30,
    SYSCON_CTRL_HR_POWER = 0x34,
    SYSCON_CTRL_LED = 0x47,
};

int syscon_issue_command_write(enum SysconCommand cmd, const unsigned char *data, unsigned int length);
int syscon_issue_command_read(enum SysconCommand cmd, unsigned char *data);
int syscon_issue_command_read_write(enum SysconCommand cmd, const unsigned char *data, unsigned int length, unsigned char *out_data);
