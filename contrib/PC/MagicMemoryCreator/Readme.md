# pspdecrypt
A quick and *dirty* tool to decrypt PSP binaries, and also PSP updaters (PSAR format)

Licensed under GPLv3

Decryption code copied from [ppsspp](https://github.com/hrydgard/ppsspp/), making use of libkirk by draan

KL3E & KL4E implementation and PSAR extraction by artart78

## Usage
`pspdecrypt` is capable of decrypting `PRX` and `IPL` files as well as decrypting and extracting `PSAR` archives and its contents, including IPL stages where possible.

## Release Notes
### 1.0
 * Merges `pspdecrypt` and `psardecrypt` into one binary
 * Top-level utility re-write with additional options by @artart78
 * syscon key ipl xor support by @proximav
 * Disable broken KIRK1 ECDSA signature verification that was slowing down extraction for certain OFW
 * Support for decrypting remaining IPL variants
 * Replace Table decryption with DES implementation
### 0.8
 * Adds KL3E & KL4E decompression support for PSAR contents
 * Adds `PSAR` support
 * Extracts most public FW, older JigKick payloads, and most TT FW
 
### Initial release (unversioned)
 * Decrypts `PRX` files
