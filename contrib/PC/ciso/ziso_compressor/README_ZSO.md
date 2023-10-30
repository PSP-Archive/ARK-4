# ZSO Format information

ZSO is an alternative to the CSO version 1 format in the PSP. It uses the LZ4 compression algorithm instead the deflate algorithm to reduce the resources usage and improve the decompression speed, at cost of some compression ratio. This format was experimental but now can be considered as stable.

The format consist in a header, an index, and the data blocks. Additionally the "magic" bytes are **ZISO** and the preferred extension is **zso**.

## Header

The main header is similar to the CSO version 1 header, and is composed by the following (little endian):

| Position | Type     | Size    | Default Value | Description                          |
|----------|----------|---------|---------------|--------------------------------------|
| 0x00     |   char   | 4 bytes |      ZISO     | Magic bytes. Always ZISO.            |
| 0x04     | uint32_t | 4 bytes |      0x18     | The header size. Always 0x18.        |
| 0x08     | uint64_t | 8 bytes |      N/A      | ISO original size                    |
| 0x10     | uint32_t | 4 bytes |      N/A      | Block size. Usually 2048.            |
| 0x14     | uint8_t  | 1 byte  |       1       | Version of the ZSO format. Always 1. |
| 0x15     | uint8_t  | 1 byte  |      N/A      | Left shift of index values.          |
| 0x16     | uint8_t  | 2 byte  |      N/A      | Unused.                              |

## Index

Following the header data there are the index entries. Those entries are composed by values in **uint32_t** (little endian), in which the **first byte (high byte)** indicates when the block is uncompressed, and the other **31 bits** are used to store the block position.

The number of index entries can be determined using the ISO original size and the Block size, using the following formula:

```
ceil(uncompressed_size / block_size) + 1
```

The extra block will be used to store the *EOF* position.

As you will have noticed the 31 index bits limits the size to at most 2GB. This is not a problem in the PSP games because the UMD size is at most 1.8GB, but with bigger image files we will have to use the **shift** header entry.

The shift value is the number of bits that we will shift right to be able to store the position into the 31 index bits, for example:

```
3.874.765.689 = 1110 0110 1111 0100 0011 1011 0111 1001
```

We will shift right the binary data:

```
0111 0011 0111 1010 0001 1101 1011 1100
```

Now we have the 31 bits, but the problem is that there is a precission lost, so we will have to "ceil" the result:

```
0111 0011 0111 1010 0001 1101 1011 1100 = 1.937.382.844

The last bit was 1, so we will "ceil" the number:

1.937.382.844 + 1 = 1.937.382.845 = 0111 0011 0111 1010 0001 1101 1011 1101
```

Padding left the result will give us a new position, that we will have to use as real start point for the block data:

```
0111 0011 0111 1010 0001 1101 1011 1101 << 1 = 3.874.765.690
3.874.765.690 - 3.874.765.689 = -1
```

The difference between the original block position and the correct position, must be padded using any byte value (usually 0x00).

A c++ example code to do the above:

```
void file_align(std::fstream &fOut, uint8_t shift)
{
    uint16_t paddingLostBytes = fOut.tellp() % (1 << shift);
    if (paddingLostBytes)
    {
        uint16_t alignment = (1 << shift) - paddingLostBytes;
        for (uint64_t i = 0; i < alignment; i++)
        {
            fOut.write("\0", 1);
        }
    }
}
```

## Data

The data blocks consist in the LZ4 compressed data. To save some space we can store the RAW uncompressed data when the compressed data is bigger, setting the **uncompressed** bit in the index entry.

## LZ4HC

The LZ4HC format is not standard in the LZO file container, so we can use it but will not be compatible with some programs or hardwares. Modern LZ4 libraries are able to decompress the LZ4HC without problem using the standard library, so modern emulators for sure will be compatible with LZ4HC format.

I personally don't recommend to use the LZ4HC algorithm, because the saved space don't worth the compatibility loose.