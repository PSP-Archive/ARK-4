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

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <ark.h>
#include <systemctrl.h>
#include "systemctrl_private.h"
#include "imports.h"

//#define CHECK_MODE
#undef CHECK_MODE

// Original Function Pointer
int (* msstorRead)(PspIoDrvFileArg * arg, char * data, int len) = NULL;
int (* msstorWrite)(PspIoDrvFileArg * arg, const char * data, int len) = NULL;
SceOff (* msstorLseek)(PspIoDrvFileArg * arg, SceOff ofs, int whence) = NULL;
int(* msstorOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) = NULL;

#ifdef DEBUG
// Cache Statistic
unsigned int cacheReadTimes = 0;
unsigned int cacheHit = 0;
unsigned int cacheMissed = 0;
unsigned int cacheUncacheable = 0;
#endif

#define MSCACHE_SIZE (4*1024)

// Cache Structure
struct MsCache
{
    char * buf;
    int bufSize;
    SceOff pos; // -1 = invalid
    int age;
};

// Cache Isntance
static struct MsCache g_cache;
static PspIoDrv* hooked_drv = NULL;
static SceUID cache_mem = -1;

// Cache Size
int g_cacheSize = 0;

// Check Range Overlapping
static inline int isCacheWithinRange(SceOff pos, SceOff start, int len)
{
    // In Range
    if(pos >= start && pos < start + len) return 1;
    
    // Out of Range
    return 0;
}

// Get Hit Cache
static struct MsCache * getHitCache(SceOff pos, int len)
{
    // Valid Position
    if(g_cache.pos != -1)
    {
        // In Range
        if(isCacheWithinRange(pos, g_cache.pos, g_cache.bufSize) && isCacheWithinRange(pos+len-1, g_cache.pos, g_cache.bufSize))
        {
            // Return Cache Reference
            return &g_cache;
        }
    }
    
    // Invalid Position
    return NULL;
}

// Disable Cache
static void disableCache(struct MsCache * cache)
{
    cache->pos = -1;
    cache->bufSize = 0;
}

// Disable Cache within Range
static void disableCacheWithinRange(SceOff pos, int len)
{
    // Valid Position
    if(g_cache.pos != -1)
    {
        // Hits our Cache Region
        if (!(pos + len <= g_cache.pos || pos >= g_cache.pos + g_cache.bufSize))
        {
            // Disable Cache
            disableCache(&g_cache);
        }
    }
}

// sceIoRead Hook
static int msstorReadCache(PspIoDrvFileArg * arg, char * data, int len)
{
    // Result
    int result = 0;
    
    // Read Length
    int read_len = 0;
    
    // Get Position in File
    SceOff pos = msstorLseek(arg, 0, PSP_SEEK_CUR);
    
    // Get Hit Cache
    struct MsCache * cache = getHitCache(pos, len);
    
    // Found Cached Portion of File
    if(cache != NULL)
    {
#ifndef CHECK_MODE
        // Calculate Buffered File Content Size
        read_len = MIN(len, cache->pos + cache->bufSize - pos);
        
        // Copy Buffered Data
        memcpy(data, cache->buf + pos - cache->pos, read_len);
        
        // Set Result
        result = read_len;
        
        // Move Position in File
        msstorLseek(arg, pos + result, PSP_SEEK_SET);
#else
        // Check validate code
        result = (*msstorRead)(arg, data, len);

        if(0 != memcmp(data, cache->buf + pos - cache->pos, len))
        {
            #ifdef DEBUG
            printk("%s: 0x%08X <%d> cache mismatched!!!\r\n", __func__, (uint)pos, (int)len);
            #endif
            _sw(0, 0);
        }
#endif

        #ifdef DEBUG        
        // Log cacheable data
        cacheHit += len;
        #endif
    }
    
    // No Cache available
    else
    {
        // Get Global Cache
        cache = &g_cache;
        
        // Requested Length fits into Cache
        if(len <= g_cacheSize)
        {
            // Disable Cache
            disableCache(cache);
            
            // Read Data
            result = msstorRead(arg, cache->buf, g_cacheSize);
            
            // Read Success
            if(result > 0)
            {
                // Save Cache Buffer Size
                read_len = result;
                // Only give the Caller the Data he wants
                result = MIN(len, result);
                
                // Copy Data into Cache
                memcpy(data, cache->buf, result);
                
                // Copy File Position into Cache
                cache->pos = pos;
                
                // Copy Cache Buffer Size into Cache
                cache->bufSize = read_len;
                
                // Forward Call
                msstorLseek(arg, pos + result, PSP_SEEK_SET);
            }
            
            #ifdef DEBUG
            // Log caching length
            cacheMissed += len;
            #endif
        }
        
        // Too big to cache...
        else
        {
            // Forward Call
            result = msstorRead(arg, data, len);
            
            #ifdef DEBUG
            // Log uncacheable data
            cacheUncacheable += len;
            #endif
        }
    }

    #ifdef DEBUG    
    // Log read data
    cacheReadTimes += len;
    #endif
    
    // Return Result
    return result;
}

// sceIoWrite Hook
static int msstorWriteCache(PspIoDrvFileArg * arg, const char * data, int len)
{
    // Get Position in File
    SceOff pos = msstorLseek(arg, 0, PSP_SEEK_CUR);
    
    // Disable Cache in Range
    disableCacheWithinRange(pos, len);
    
    // Forward Call
    return msstorWrite(arg, data, len);
}

// sceIoOpen Hook
static int msstorOpenCache(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode)
{
    // New File = New Cache
    disableCache(&g_cache);
    
    // Forward Call
    return msstorOpen(arg, file, flags, mode);
}

static int (*msstorIoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
static int msstorIoIoctlCache(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    disableCache(&g_cache);
    return msstorIoIoctl(arg, cmd, indata, inlen, outdata, outlen);
}

static int (*msstorIoRemove)(PspIoDrvFileArg *arg, const char *name);
static int msstorIoRemoveCache(PspIoDrvFileArg *arg, const char *name)
{
    disableCache(&g_cache);
    return msstorIoRemove(arg, name);
}

static int (*msstorIoMkdir)(PspIoDrvFileArg *arg, const char *name, SceMode mode);
static int msstorIoMkdirCache(PspIoDrvFileArg *arg, const char *name, SceMode mode)
{
    disableCache(&g_cache);
    return msstorIoMkdir(arg, name, mode);
}

static int (*msstorIoRmdir)(PspIoDrvFileArg *arg, const char *name);
static int msstorIoRmdirCache(PspIoDrvFileArg *arg, const char *name)
{
    disableCache(&g_cache);
    return msstorIoRmdir(arg, name);
}

static int (*msstorIoDopen)(PspIoDrvFileArg *arg, const char *dirname);
static int msstorIoDopenCache(PspIoDrvFileArg *arg, const char *dirname)
{
    disableCache(&g_cache);
    return msstorIoDopen(arg, dirname);
}

static int (*msstorIoDclose)(PspIoDrvFileArg *arg);
static int msstorIoDcloseCache(PspIoDrvFileArg *arg)
{
    disableCache(&g_cache);
    return msstorIoDclose(arg);
}

static int (*msstorIoDread)(PspIoDrvFileArg *arg, SceIoDirent *dir);
static int msstorIoDreadCache(PspIoDrvFileArg *arg, SceIoDirent *dir)
{
    disableCache(&g_cache);
    return msstorIoDread(arg, dir);
}

static int (*msstorIoGetstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);
static int msstorIoGetstatCache(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat)
{
    disableCache(&g_cache);
    return msstorIoGetstat(arg, file, stat);
}

static int (*msstorIoChstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits);
static int msstorIoChstatCache(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits)
{
    disableCache(&g_cache);
    return msstorIoChstat(arg, file, stat, bits);
}

static int (*msstorIoRename)(PspIoDrvFileArg *arg, const char *oldname, const char *newname);
static int msstorIoRenameCache(PspIoDrvFileArg *arg, const char *oldname, const char *newname)
{
    disableCache(&g_cache);
    return msstorIoRename(arg, oldname, newname);
}

static int (*msstorIoChdir)(PspIoDrvFileArg *arg, const char *dir);
static int msstorIoChdirCache(PspIoDrvFileArg *arg, const char *dir)
{
    disableCache(&g_cache);
    return msstorIoChdir(arg, dir);
}

static int (*msstorIoMount)(PspIoDrvFileArg *arg);
static int msstorIoMountCache(PspIoDrvFileArg *arg)
{
    disableCache(&g_cache);
    return msstorIoMount(arg);
}

static int (*msstorIoUmount)(PspIoDrvFileArg *arg);
static int msstorIoUmountCache(PspIoDrvFileArg *arg)
{
    disableCache(&g_cache);
    return msstorIoUmount(arg);
}

static int (*msstorIoDevctl)(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
static int msstorIoDevctlCache(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    disableCache(&g_cache);
    return msstorIoDevctl(arg, devname, cmd, indata, inlen, outdata, outlen);
}

static int (*msstorIoUnk21)(PspIoDrvFileArg *arg);
static int msstorIoUnk21Cache(PspIoDrvFileArg *arg)
{
    disableCache(&g_cache);
    return msstorIoUnk21(arg);
}

// Initialize "ms" Driver Cache
int msstorCacheInit(const char* driver)
{

    if (driver == NULL){
        if (hooked_drv){
            // Unhook Driver Functions
            hooked_drv->funcs->IoRead = msstorRead;
            hooked_drv->funcs->IoWrite = msstorWrite;
            hooked_drv->funcs->IoOpen= msstorOpen;
            hooked_drv->funcs->IoIoctl = msstorIoIoctl;
            hooked_drv->funcs->IoRemove = msstorIoRemove;
            hooked_drv->funcs->IoMkdir = msstorIoMkdir;
            hooked_drv->funcs->IoRmdir = msstorIoRmdir;
            hooked_drv->funcs->IoDopen = msstorIoDopen;
            hooked_drv->funcs->IoDclose = msstorIoDclose;
            hooked_drv->funcs->IoDread = msstorIoDread;
            hooked_drv->funcs->IoGetstat = msstorIoGetstat;
            hooked_drv->funcs->IoChstat = msstorIoChstat;
            hooked_drv->funcs->IoRename = msstorIoRename;
            hooked_drv->funcs->IoChdir = msstorIoChdir;
            hooked_drv->funcs->IoMount = msstorIoMount;
            hooked_drv->funcs->IoUmount = msstorIoUmount;
            hooked_drv->funcs->IoDevctl = msstorIoDevctl;
            hooked_drv->funcs->IoUnk21 = msstorIoUnk21;
        }
        sceKernelFreePartitionMemory(cache_mem);
        cache_mem = -1;
        g_cache.buf = NULL;
        g_cache.bufSize = 0;
        return 0;
    }

    if (g_cacheSize > 0) return 0; // cache already on
    if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) return 0; // not needed on POPS

    // Get Application Type
    int key_config = sceKernelApplicationType();
    
    // Find Driver
    PspIoDrv * pdrv = sctrlHENFindDriver(driver);
    
    // Driver unavailable
    if(pdrv == NULL) return -1;
    
    // Allocate Memory
    SceUID memid = sceKernelAllocPartitionMemory(1, "MsStorCache", PSP_SMEM_High, MSCACHE_SIZE + 64, NULL);
    cache_mem = memid;
    
    // Allocation failed
    if(memid < 0) return -3;
    
    // Get Memory Pointer
    g_cache.buf = sceKernelGetBlockHeadAddr(memid);
    
    // Couldn't fetch Pointer
    if(g_cache.buf == NULL) return -4;
    
    // Align Buffer to 64 Byte
    g_cache.buf = (void *)(((unsigned int)g_cache.buf & (~(64-1))) + 64);
    
    // Set Cache Size
    g_cacheSize = MSCACHE_SIZE;
    
    // Disable Cache
    disableCache(&g_cache);
    
    // Fetch Driver Functions
    hooked_drv = pdrv;
    msstorRead = pdrv->funcs->IoRead;
    msstorWrite = pdrv->funcs->IoWrite;
    msstorLseek = pdrv->funcs->IoLseek;
    msstorOpen = pdrv->funcs->IoOpen;
    msstorIoIoctl = pdrv->funcs->IoIoctl;
    msstorIoRemove = pdrv->funcs->IoRemove;
    msstorIoMkdir = pdrv->funcs->IoMkdir;
    msstorIoRmdir = pdrv->funcs->IoRmdir;
    msstorIoDopen = pdrv->funcs->IoDopen;
    msstorIoDclose = pdrv->funcs->IoDclose;
    msstorIoDread = pdrv->funcs->IoDread;
    msstorIoGetstat = pdrv->funcs->IoGetstat;
    msstorIoChstat = pdrv->funcs->IoChstat;
    msstorIoRename = pdrv->funcs->IoRename;
    msstorIoChdir = pdrv->funcs->IoChdir;
    msstorIoMount = pdrv->funcs->IoMount;
    msstorIoUmount = pdrv->funcs->IoUmount;
    msstorIoDevctl = pdrv->funcs->IoDevctl;
    msstorIoUnk21 = pdrv->funcs->IoUnk21;
    
    // Hook Driver Functions
    if (msstorRead) pdrv->funcs->IoRead = msstorReadCache;
    if (msstorWrite) pdrv->funcs->IoWrite = msstorWriteCache;
    if (msstorOpen) pdrv->funcs->IoOpen= msstorOpenCache;
    if (msstorIoIoctl) pdrv->funcs->IoIoctl = msstorIoIoctlCache;
    if (msstorIoRemove) pdrv->funcs->IoRemove = msstorIoRemoveCache;
    if (msstorIoMkdir) pdrv->funcs->IoMkdir = msstorIoMkdirCache;
    if (msstorIoRmdir) pdrv->funcs->IoRmdir = msstorIoRmdirCache;
    if (msstorIoDopen) pdrv->funcs->IoDopen = msstorIoDopenCache;
    if (msstorIoDclose) pdrv->funcs->IoDclose = msstorIoDcloseCache;
    if (msstorIoDread) pdrv->funcs->IoDread = msstorIoDreadCache;
    if (msstorIoGetstat) pdrv->funcs->IoGetstat = msstorIoGetstatCache;
    if (msstorIoChstat) pdrv->funcs->IoChstat = msstorIoChstatCache;
    if (msstorIoRename) pdrv->funcs->IoRename = msstorIoRenameCache;
    if (msstorIoChdir) pdrv->funcs->IoChdir = msstorIoChdirCache;
    if (msstorIoMount) pdrv->funcs->IoMount = msstorIoMountCache;
    if (msstorIoUmount) pdrv->funcs->IoUmount = msstorIoUmountCache;
    if (msstorIoDevctl) pdrv->funcs->IoDevctl = msstorIoDevctlCache;
    if (msstorIoUnk21) pdrv->funcs->IoUnk21 = msstorIoUnk21Cache;
    
    // Return Success
    return 0;
}

// For PSPLink Debugging
// call @SystemControl:SystemCtrlPrivate,0xFFC9D099@
void msstorCacheStat(int reset)
{
    #ifdef DEBUG
    // Output Buffer
    char buf[256];
    
    // Statistic available
    if(cacheReadTimes != 0)
    {
        // Output to Stdout
        sprintf(buf, "Mstor cache size: %dKB\n", g_cacheSize / 1024);
        sceIoWrite(1, buf, strlen(buf));
        sprintf(buf, "hit percent: %02d%%/%02d%%/%02d%%, [%d/%d/%d/%d]\n", 
                (int)(100 * cacheHit / cacheReadTimes), 
                (int)(100 * cacheMissed / cacheReadTimes), 
                (int)(100 * cacheUncacheable / cacheReadTimes), 
                (int)cacheHit, (int)cacheMissed, (int)cacheUncacheable, (int)cacheReadTimes);
        sceIoWrite(1, buf, strlen(buf));
        sprintf(buf, "caches stat:\n");
        sceIoWrite(1, buf, strlen(buf));
        sprintf(buf, "Cache Pos: 0x%08X bufSize: %d Buf: 0x%08X\n", (uint)g_cache.pos, g_cache.bufSize, (uint)g_cache.buf);
        sceIoWrite(1, buf, strlen(buf));
    }
    
    // No Statistic available
    else
    {
        // Output to Stdout
        sprintf(buf, "no msstor cache call yet\n");
        sceIoWrite(1, buf, strlen(buf));
    }
    
    // Statistic Reset requested
    if(reset)
    {
        // Delete Statistic
        cacheReadTimes = cacheHit = cacheMissed = cacheUncacheable = 0;
    }
    #endif
}

// For PSPLink Debugging
// call @SystemControl:SystemCtrlPrivate,0x657301D9@
void msstorCacheDisable(void)
{
    // Disable Cache
    disableCache(&g_cache);
}

