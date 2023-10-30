# ZISO compressor

This ziso compressor is an alternative to the ziso.py compressor that I have found in the OPL repository. It doesn't respect the compression level and doesn't provide LZ4HC compression, so I have decided to create my own compressor.

## Features

* Allows to set the compression level by using the LZ4 feature called "acceleration".
* Includes the LZ4HC compression algorithm.
* It includes an alternative compression method which can reduce the size in some cases.
* Brute force compression to use the best compression method between the two LZ4 methods.
* It's 40-45% faster than the ziso.py conversor.
* It's able to detect CD-ROM images and adjust the blocksize according with their sector size.

## ToDo

There are some things I'd like to do, like for example:

* ~~Try to add read and write buffers to improve the speed~~ -> 50-55% faster compressing and up to 70% decompressing.
* ~~CD-ROM detection to select the best block size (DVD -> 2048 vs CD-ROM -> 2352)~~ -> Done. Now the program detects CDROM images.
* Add alternative compressor libraries like [smalLZ4](https://github.com/stbrumme/smallz4) and [lz4ultra](https://github.com/emmanuel-marty/lz4ultra). They are supposed to get better compression ratio.
* Add Multi Thread processing.
* Maybe add an extra step to analyze the ISO. The space saving in the best scenario is about 4,5MB, so will be the lower priority.

## Compile

Clone the repository:

```
git clone git@github.com:Danixu/ziso_compressor.git
```

Download the submodules:

```
cd ziso_compressor
git submodules update --init
```

Compile the program:

This repository includes the cmake configuration, so to compile you will have to run the standard commands:

```
$ cmake -B .
$ make
```

This will create the binary files into the bin directory.

Tested in Manjaro with the standard build tools, and Windows 11 with MSYS2 with MinGW64.

## Usage

The program is easy to use, and the output filename will be determined if not provided. Also it is able to detect the ZISO files, so will determine if the file must be compressed or uncompressed.

Compress/decompress a file:

```
ziso.exe -i <input-file>

ziso.exe -i <input-file> -o <output-file>
```

## Arguments

| Short |      Long     | Value |                      Description                                 |
|:-----:|:-------------:|:-----:|:----------------------------------------------------------------:|
|   -i  | --input       |       | Input file to process                                            |
|   -o  | --output      |       | Output file                                                      |
|   -c  | --compression |  1-12 | Compression level                                                |
|   -m  | --mode2-lz4   |       | Use an alternative LZ4 compression method                        |
|   -l  | --lz4hc       |       | Activate the High Compression algorithm                          |
|   -f  | --brute-force |       | Test the two LZ4 compression methods and use the best            |
|   -b  | --block-size  |  2048 | Block size, usually 2048 but better 2352 for CD-ROMS             |
|   -z  | --cache-size  |   4   | Cache size in MB to improve the compression/decompression speed  |
|   -r  | --replace     |       | Force to overwrite the output file                               |
|   -h  | --hdl-fix     |       | hdl_dump fix when copied to internal PS2 HDD                     |


### Explanation

#### Compression

The LZ4 method doesn't have compression level arguments. Instead, it has **acceleration** which will affects the speed and compression ratio (just like the compression arguments). On my program I use the **acceleration** to supply the compression level.

The LZ4HC method already includes the compression option, so it will passed to it directly.

#### Alternative LZ4 compression method

The LZ4 library has two ways to compress the data. One is intended to be used to compress the data at once, and the other to compress several blocks of data keeping the dictionary to improve the compression ratio.

In this case both are used in the same way: compress all the data at once, but for any reason there are some differences between both even when they uses the same algorithm. Sometimes the first method compress better and sometimes the 2nd. That is why I provide the "alternative" method to be used when it compress a bit better.

The default compression method produces the same output as the ziso.py program.

#### LZ4 High Compression

The LZ4 library provides an alternative compression method called LZ4HC, which provides a better compression ratio in most scenarios. It is also slower and some programs and hardwares can be incompatible with this compression method. That is why I only recommend to use if when we are fully sure that the reader is compatible. For example, the ziso.py compressor is able to decompress this files even when it doesn't have the compression method. Also moderns emulators can be compatible with it.

#### Brute Force Search

As I have explained above, there are two compression functions in LZ4. This argument will compress every block using both and will write the smaller result to the output file.

LZ4HC will not be tried because it also uses the best compression method, so using the brute-force option with the LZ4HC compression method included will produce the same result as to use directly the lz4hc compression option. That is why is better to directly use that option unless we want to keep the compatibility with the standard LZ4 libraries without HC.

#### Block Size

Size of every compressed block. By default 2048, but it's recommended to change it to 2352 when a CD-ROM is being compressed. A bigger block size improve the compression ratio, but increases the memory required by the decompresor to get the original data. Also the reader must be compatible with the blocksize or will fail.

#### Cache Size

The size of the cache memory used as buffer to improve the compression and decompression speed. It reduces the required read and write IO request improving the compression speed up to 40-50% and the decompression speed up to 70% compared with the ziso.py version. Normally the default size is enought and increasing the size will not improve the speed, but in some cases can be a good option.

#### HDL Fix

Actually there is a [bug in the hdl_dump](https://github.com/ps2homebrew/hdl-dump/issues/71) which will trim the latest bytes of the file if the size is not a multiple of 2048. To solve it, the program will pad the output file to the nearest upper 2048 bytes multiple.