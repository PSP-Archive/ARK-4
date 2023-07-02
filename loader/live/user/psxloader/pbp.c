#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint32_t u32;

typedef struct
{
    u32 magic;
    u32 version;
    u32 param_offset;
    u32 icon0_offset;
    u32 icon1_offset;
    u32 pic0_offset;
    u32 pic1_offset;
    u32 snd0_offset;
    u32 elf_offset;
    u32 psar_offset;
} PBPHeader;

int main(){

    FILE* fp = fopen("EBOOT.PBP", "rb");
    
    FILE* out = fopen("simple.prx", "wb");
    
    PBPHeader header;
    fread(&header, 1, sizeof(header), fp);
    
    u32 size = header.psar_offset - header.elf_offset;

    if (size){
        void* data = malloc(size);
        fseek(fp, header.elf_offset, SEEK_SET);
        fread(data, 1, size, fp);
        fwrite(data, 1, size, out);
    }

    fclose(fp);
    fclose(out);

    return 0;

}
