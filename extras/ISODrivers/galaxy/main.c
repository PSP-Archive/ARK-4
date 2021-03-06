/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspthreadman_kernel.h>
#include <psputilsforkernel.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <globals.h>
#include <functions.h>
#include "imports.h"
#include "galaxy.h"
#include "ansi_c_functions.h"
#include "psid.h"
#include "lz4.h"

PSP_MODULE_INFO("GalaxyController", 0x1006, 1, 1);

// 6.60 np9660.prx
#define NP9660_INIT_FOR_KERNEL_CALL 0x3C5C
#define NP9660_INIT_ISOFS_CALL 0x3C78
#define NP9660_READ_DISC_SECTOR_CALL_1 0x4414
#define NP9660_READ_DISC_SECTOR_CALL_2 0x596C
#define NP9660_IO_CLOSE_STUB 0x7D68
#define NP9660_INIT 0x36A8
// These need code upgrade to point to BSS properly, right now they are relative to Text
// See 0x3394
// NP9660_ISO_FD at the start of BSS
#define NP9660_DATA_1 (0x00005BB4 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_2 (0x00005BBC - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_3 (0x00005BD0 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_4 (0x00005BD8 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_5 (0x00000114 + 0x00008900)
#define NP9660_ISO_FD (0x00000188 + 0x00008900)
#define NP9660_READ_DISC_SECTOR 0x4FEC
#define NP9660_READ_SECTOR_FLUSH 0x505C

// SceNpUmdMount Thread ID
SceUID g_SceNpUmdMount_thid = -1;

// ISO File Descriptor
SceUID g_iso_fd = -1;

// ISO Block Count
int g_total_blocks = -1;

// ISO Open Flag
int g_iso_opened = 0;

// CSO Mode Flag
int g_is_ciso = 0;

// CSO Block Buffer
void * g_ciso_block_buf = NULL;

// CSO Decompress Buffer
void * g_ciso_dec_buf = NULL;

// CSO Index Cache
unsigned int g_CISO_idx_cache[CISO_IDX_BUFFER_SIZE / 4] __attribute__((aligned(64)));

// CSO Decompress Buffer Offset
unsigned int g_ciso_dec_buf_offset = (unsigned int)-1;

// CSO Decompress Buffer Size
int g_ciso_dec_buf_size = 0;

// CSO Current Index
int g_CISO_cur_idx = 0;

// CSO Header
static struct CISO_header g_CISO_hdr __attribute__((aligned(64)));

// CSO Block Count
unsigned int ciso_total_block = 0;

// np9660.prx Text Address
unsigned int g_sceNp9660_driver_text_addr = 0;

// ISO File Path
char * g_iso_fn = NULL;

// is ZSO
static int lz4_compressed = 0;

// Dummy UMD Data for Global Pointer
unsigned char g_umddata[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// NP9660 Initialization Call
void (* initNP9660)(unsigned int zero) = NULL;

// NP9660 Disc Sector Reader
int (* readDiscSectorNP9660)(unsigned int sector, unsigned char * buffer, unsigned int size) = NULL;

// NP9660 Sector Flush
void (* readSectorFlushNP9660)(void) = NULL;

// Get Disc Sector Count
int get_total_block(void)
{
    // CSO File
    if(g_is_ciso)
    {
        // Return CSO Header Value
        return ciso_total_block;
    }
    
    // Move to End of ISO File
    SceOff offset = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);
    
    // Return Error
    if(offset < 0) return (int)offset;
    
    // Return Sector Count
    return offset / ISO_SECTOR_SIZE;
}

// Read Raw ISO Data
int read_raw_data(unsigned char * addr, unsigned int size, unsigned int offset)
{
    // Retry Counter
    int i = 0;
    
seek:
    // Seek to Position
    for(i = 0; i < 16; i++)
    {
        // Seek to Position
        SceOff ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);
        
        // Seek Success
        if(ofs >= 0) break;
        
        // Reopen ISO File
        open_iso();
    }
    
    // Seek Error
    if(i == 16) return 0x80010013;
    
    // Read Data
    int read = sceIoRead(g_iso_fd, addr, size);
    
    // Read Failure
    if(read < 0)
    {
        // Reopen ISO File
        open_iso();
        
        // Retry Seeking
        goto seek;
    }
    
    // Return Read Bytes
    return read;
}

// Read CSO Disc Sector
int read_cso_sector(unsigned char * addr, int sector)
{
    // Negative Sector
    int n_sector = sector - g_CISO_cur_idx;
    
    // Uncached Sector
    if(g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache))
    {
        // Read Raw Data
        int read = read_raw_data((unsigned char *)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(struct CISO_header));
        
        // Read Failure
        if(read < 0) return -4;
        
        // Set Current Sector Index
        g_CISO_cur_idx = sector;
        
        // Reset Negative Sector
        n_sector = 0;
    }
    
    // Calculate Offset
    unsigned int offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;
    
    // Plain Sector
    if(g_CISO_idx_cache[n_sector] & 0x80000000)
    {
        // Read Raw Data
        return read_raw_data(addr, ISO_SECTOR_SIZE, offset);
    }
    
    // Increase Sector Number
    sector++;
    
    // Negative Sector
    n_sector = sector - g_CISO_cur_idx;
    
    // Uncached Sector
    if(g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache))
    {
        // Read Raw Data
        int read = read_raw_data((unsigned char *)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(struct CISO_header));
        
        // Read Failure
        if(read < 0) return -5;
        
        // Set Current Sector Index
        g_CISO_cur_idx = sector;
        
        // Reset Negative Sector
        n_sector = 0;
    }
    
    // Calculate Next Offset
    unsigned int next_offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;
    
    // Calculate Size
    unsigned int size = next_offset - offset;
    
    // Alignment Required
    if(g_CISO_hdr.align) size += 1 << g_CISO_hdr.align;
    
    // Set Minimum Size
    if(size <= ISO_SECTOR_SIZE) size = ISO_SECTOR_SIZE;
    
    // Packed Payload required
    if(g_ciso_dec_buf_offset == (unsigned int)-1 || offset < g_ciso_dec_buf_offset || offset + size >= g_ciso_dec_buf_offset + g_ciso_dec_buf_size)
    {
        // Read Raw Data
        int read = read_raw_data(g_ciso_dec_buf, size, offset);
        
        // Broken Payload Buffer
        if(read < 0)
        {
            // Set Buffer Offset
            g_ciso_dec_buf_offset = 0xFFF00000;
            
            // Read Failure
            return -6;
        }
        
        // Set Buffer Offset
        g_ciso_dec_buf_offset = offset;
        
        // Set Buffer Size
        g_ciso_dec_buf_size = read;
    }
    
    // Unpack Data
    int read;
    if (lz4_compressed){
        read = LZ4_decompress_fast(g_ciso_dec_buf + offset - g_ciso_dec_buf_offset, addr, ISO_SECTOR_SIZE);
        if(read < 0) {
            read = -20;
        }
    }
    else {
        read = sceKernelDeflateDecompress(addr, ISO_SECTOR_SIZE, g_ciso_dec_buf + offset - g_ciso_dec_buf_offset, 0);
    }
    // Return Result
    return read < 0 ? read : ISO_SECTOR_SIZE;
}

// Custom CSO Sector Reader
int read_cso_data(unsigned char * addr, unsigned int size, unsigned int offset)
{
    // Backup Start Sector Offset
    unsigned int o_offset = offset;
    
    // Reading Loop
    while(size > 0)
    {
        // Calculate Disc Sector Number
        unsigned int cur_block = offset / ISO_SECTOR_SIZE;
        
        // Calculate Relative Position in Sector
        unsigned int pos = offset & (ISO_SECTOR_SIZE - 1);
        
        // End of File
        if(cur_block >= g_total_blocks) break;
        
        // Read Disc Sector
        int read = read_cso_sector(g_ciso_block_buf, cur_block);
        
        // Read Error
        if(read != ISO_SECTOR_SIZE) return -7;
        
        // Calculate Read Bytes (considering Sector Size)
        read = MIN(size, (ISO_SECTOR_SIZE - pos));
        
        // Copy Data into Output Buffer
        memcpy(addr, g_ciso_block_buf + pos, read);
        
        // Reduce remaining Size
        size -= read;
        
        // Move Output Buffer
        addr += read;
        
        // Advance Offset
        offset += read;
    }
    
    // Return Read Size
    return offset - o_offset;
}

// Custom ISO Sector Reader Wrapper
int iso_read(struct IoReadArg * args)
{
    // Compressed ISO
    if(g_is_ciso != 0)
    {
        // Read Data
        return read_cso_data(args->address, args->size, args->offset);
    }
    
    // Uncompressed ISO
    else
    {
        // Read Data
        return read_raw_data(args->address, args->size, args->offset);
    }
}

// Open CSO File (subcall from open_iso)
int cso_open(SceUID fd)
{
    // Reset CSO Information
    g_CISO_hdr.magic[0] = 0;
    g_ciso_dec_buf_offset = (unsigned int)-1;
    g_ciso_dec_buf_size = 0;
    
    // Rewind File
    sceIoLseek(fd, 0, PSP_SEEK_SET);
    
    // Read CSO Header
    if(sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr)) != sizeof(g_CISO_hdr))
    {
        // Read Error
        return -1;
    }
    
    // Invalid "CSO/ZSO" Magic
    u32 magic = _lw((unsigned int)g_CISO_hdr.magic);
    if(magic != 0x4F534943 && magic != 0x4F53495A) return 0x8002012F;
    
    lz4_compressed = (magic == 0x4F53495A);
    
    // Reset CSO Index
    g_CISO_cur_idx = -1;
    
    // Calculate Sector Count
    ciso_total_block = g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;
    
    // Decompression Buffer not yet created
    if(g_ciso_dec_buf == NULL)
    {
        // Allocate Kernel Memory from System Control
        g_ciso_dec_buf = oe_malloc(CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align) + 64);
        
        // Allocation Error
        if(g_ciso_dec_buf == NULL) return -2;
        
        // Alignment required
        if(((unsigned int)g_ciso_dec_buf & 63) != 0)
        {
            // Align Buffer
            g_ciso_dec_buf = (void *)(((unsigned int)g_ciso_dec_buf & (~63)) + 64);
        }
    }
    
    // Sector Buffer not yet created
    if(g_ciso_block_buf == NULL)
    {
        // Allocate Kernel Memory from System Control
        g_ciso_block_buf = oe_malloc(ISO_SECTOR_SIZE);
        
        // Allocation Error
        if(g_ciso_block_buf == NULL) return -3;
    }
    
    // Return Success
    return 0;
}

// Open ISO File
int open_iso(void)
{
    // Close ISO (if open)
    sceIoClose(g_iso_fd);
    
    // Erase Global Flag
    g_iso_opened = 0;
    
    // Open Loop
    while(1)
    {
        // Open ISO
        g_iso_fd = sceIoOpen(g_iso_fn, 0xF0000 | PSP_O_RDONLY, 0);
        
        // Opened ISO
        if(g_iso_fd >= 0) break;
        
        // Wait and repeat
        sceKernelDelayThread(10000);
    }
    
    // Update NP9660 File Descriptor Variable
    _sw(g_iso_fd, g_sceNp9660_driver_text_addr + NP9660_ISO_FD);
    
    // Reset CISO Flag
    g_is_ciso = 0;
    
    // Attempt CSO File Open
    if(cso_open(g_iso_fd) >= 0)
    {
        // Set CISO Flag
        g_is_ciso = 1;
    }
    
    // Update Disc Block Count
    g_total_blocks = get_total_block();
    
    // Set Global Flag
    g_iso_opened = 1;
    
    // Return Success
    return 0;
}

// readDiscSectorNP9660 Hook
int readDiscSector(unsigned int sector, unsigned char * buffer, unsigned int size)
{
    // Forward Call (this will fail but it sets some variables we need)
    readDiscSectorNP9660(sector, buffer, size);
    
    // Prepare Arguments for our custom ISO Sector Reader
    struct IoReadArg args;
    args.offset = sector;
    args.address = buffer;
    args.size = size;
    
    // Forward Call to our custom ISO Sector Reader
    int result = sceKernelExtendKernelStack(0x2000, (void *)iso_read, &args);
    
    // Flush & Update NP9660 Sector Information
    readSectorFlushNP9660();
    
    // Return Result
    return result;
}

// sceIoClose Hook
int myIoClose(int fd)
{
    // Forward Call
    int result = sceIoClose(fd);
    
    // ISO was closed
    if(fd == g_iso_fd)
    {
        // Remove ISO File Descriptor
        g_iso_fd = -1;
        
        // Remove File Descriptor
        _sw(-1, g_sceNp9660_driver_text_addr + NP9660_ISO_FD);
        
        // Flush Cache
        flushCache();
    }
    
    // Return Result
    return result;
}

// Init ISO Emulator
int initEmulator(void)
{
    // First Run
    static int firstrun = 1;
    
    // Initialize NP9660
    initNP9660(0);
    
    // Open ISO File
    open_iso();
    
    // Suspend Interrupts
    int interrupts = sceKernelCpuSuspendIntr();

    // sceUmdManGetUmdDiscInfo Patch
    _sw(0xE0000800, g_sceNp9660_driver_text_addr + NP9660_DATA_1);
    _sw(9, g_sceNp9660_driver_text_addr + NP9660_DATA_2);
    _sw(g_total_blocks, g_sceNp9660_driver_text_addr + NP9660_DATA_3);
    _sw(g_total_blocks, g_sceNp9660_driver_text_addr + NP9660_DATA_4);
    _sw(0, g_sceNp9660_driver_text_addr + NP9660_DATA_5);
    
    // Resume Interrupts
    sceKernelCpuResumeIntr(interrupts);
    
    // First Run Delay
    if(firstrun)
    {
        // Wait a bit...
        sceKernelDelayThread(800000);
        
        // Disable First Run Delay
        firstrun = 0;
    }
    
    // Clush Cache
    flushCache();
    
    // Set Global Pointer for UMD Data
    sceKernelSetQTGP3(g_umddata);
    
    // Return Success
    return 0;
}

// sceKernelStartThread Hook
int myKernelStartThread(SceUID thid, SceSize arglen, void * argp)
{
    // SceNpUmdMount Thread
    if(g_SceNpUmdMount_thid == thid)
    {
        // Find Driver Module
        SceModule2 * pMod = (SceModule2 *)sceKernelFindModuleByName("sceNp9660_driver");
        
        // Fetch Text Address
        g_sceNp9660_driver_text_addr = pMod->text_addr;
        
        // Make InitForKernel Call return 0x80000000 always
        _sw(0x3C028000, g_sceNp9660_driver_text_addr + NP9660_INIT_FOR_KERNEL_CALL); // jal InitForKernel_23458221 to lui $v0, 0x00008000
        
        // Hook Function #1
        _sw(JAL(initEmulator), g_sceNp9660_driver_text_addr + NP9660_INIT_ISOFS_CALL);
        
        // Hook Function #2
        _sw(JAL(readDiscSector), g_sceNp9660_driver_text_addr + NP9660_READ_DISC_SECTOR_CALL_1); // jal sub_3948 to jal sub_00000054
        _sw(JAL(readDiscSector), g_sceNp9660_driver_text_addr + NP9660_READ_DISC_SECTOR_CALL_2); // jal sub_3948 to jal sub_00000054
        
        // Hook sceIoClose Stub
        _sw(JUMP(myIoClose), g_sceNp9660_driver_text_addr + NP9660_IO_CLOSE_STUB); // hook sceIoClose import
        
        // Backup Function Pointer
        initNP9660 = (void *)(pMod->text_addr + NP9660_INIT);
        readDiscSectorNP9660 = (void *)(pMod->text_addr + NP9660_READ_DISC_SECTOR);
        readSectorFlushNP9660 = (void *)(pMod->text_addr + NP9660_READ_SECTOR_FLUSH);
        
        // Flush Cache
        flushCache();
    }
    
    // Forward Call
    return sceKernelStartThread(thid, arglen, argp);
}

// sceKernelCreateThread Hook
SceUID myKernelCreateThread(const char * name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam * option)
{
    // Forward Call
    SceUID thid = sceKernelCreateThread(name, entry, initPriority, stackSize, attr,    option);
    
    // SceNpUmdMount Thread
    if(strncmp(name, "SceNpUmdMount", 13) == 0)
    {
        // Save Thread ID
        g_SceNpUmdMount_thid = thid;
    }
    
    // Return Result
    return thid;
}

u32 findReferenceInGlobalScope(u32 ref){
    u32 addr = ref;
    while (strcmp("ThreadManForKernel", (char*)addr))
        addr++;
    addr+=18;
    
    if (addr%4)
        addr &= -0x4;
    
    while (_lw(addr += 4) != ref);
    
    return addr;
}

// Entry Point
int module_start(SceSize args, void* argp)
{

    printk("galaxy started: compiled at %s %s\r\n", __DATE__, __TIME__);
    
    // Get ISO Path
    g_iso_fn = sctrlSEGetUmdFile();
    printk("UmdFile: %s\r\n", g_iso_fn);

    // Leave NP9660 alone, we got no ISO
    if(g_iso_fn[0] == 0) return 1;
    
    // Find Thread Manager
    SceModule2 * pMod = (SceModule2 *)sceKernelFindModuleByName("sceThreadManager");
    
    // Found Thread Manager
    if(pMod != NULL)
    {
        u32 ref;
        // Hook sceKernelCreateThread
        ref = K_EXTRACT_CALL(&sceKernelCreateThread);
        _sw((unsigned int)myKernelCreateThread, findRefInGlobals("ThreadManForKernel", ref, ref));
        
        // Hook sceKernelStartThread
        ref = K_EXTRACT_CALL(&sceKernelStartThread);
        _sw((unsigned int)myKernelStartThread, findRefInGlobals("ThreadManForKernel", ref, ref));
    }
    
    // Flush Cache
    flushCache();
    
    // ISO File Descriptor
    int fd = -1;
    
    // Wait for MS
    while(1)
    {
        // Open ISO File
        fd = sceIoOpen(g_iso_fn, PSP_O_RDONLY, 0);
        
        // Open Success
        if(fd >= 0) break;
        
        // Delay and retry
        sceKernelDelayThread(10000);
    }
    
    // Close File
    sceIoClose(fd);
    
    // Start Success
    return 0;
}

