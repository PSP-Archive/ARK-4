The core files of the Custom Firmware, what gets installed on flash0.

- SystemControl: the heart of the Custom Firmware
- CompatLayer: the device driver.
- Stargate: the No-DRM engine.
- Inferno: the ISO driver.
- PopCorn: the PS1 driver.
- VshControl: the XMB driver.

All core files must follow these guidelines:

1. They must not do any static offset patching, only dynamic patching.
2. They must not do any model or firmware check (psp_model and psp_fw_version).
3. They must not do any device check (ark_config->exec_mode).



Only a few modules and parts of the kernel are allowed to break rules:

1. VshControl can break rule 2 and 3 for PSP-Go, PSP-1K and PS Vita checks.
2. CompatLayer can break all rules.
3. Externally loaded rebootex can break all rules.


These guidelines allow ARK to be as robust as possible while also working with as many different models and devices.
