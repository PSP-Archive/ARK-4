# Ziso Changelog

## v0.5.0

* First version of the program.
* Includes two LZ4 compression methods whith different results. Their result can vary depending of the source data. By default the same as ziso.py will be used.
* Brute force compression to select the best of the two methods for every block.
* LZ4HC compression is included.
* Faster than the original ziso.py compressor.
* Blocks with bigger compressed size are not compressed (like the original ziso.py tool).
* Ability to detect source CDROM to adjust the block size to their sector size (2352). This will improve the decompression speed by reducing the number of blocks to decompress to get a full sector (1 vs 2).