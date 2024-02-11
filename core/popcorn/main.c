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
#include <pspinit.h>
#include <psputilsforkernel.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <globals.h>
#include <functions.h>

extern unsigned char g_icon_png[6108];

PSP_MODULE_INFO("PROPopcornManager", 0x1007, 1, 2);

static STMOD_HANDLER g_previous = NULL;

static unsigned int g_pspFwVersion;

static int g_isCustomPBP;
static int g_keysBinFound;
static SceUID g_plain_doc_fd = -1;

#define PGD_ID "XX0000-XXXX00000_00-XXXXXXXXXX000XXX"
#define ACT_DAT "flash2:/act.dat"
#define RIF_MAGIC_FD 0x10000
#define ACT_DAT_FD 0x10001

enum {
    ICON0_OK = 0,
    ICON0_MISSING = 1,
    ICON0_CORRUPTED = 2,
};

static int g_icon0Status;

static unsigned char g_keys[16];

// Get keys.bin path
static int getKeysBinPath(char *keypath, unsigned int size);

// Save keys.bin
static int saveKeysBin(const char *keypath, unsigned char *key, int size);

static void patchPops(SceModule2 *mod);

static void popcornSyspatch(SceModule2 *mod)
{
    #ifdef DEBUG
    printk("%s: %s\r\n", __func__, mod->modname);
    #endif

    if (strcmp(mod->modname, "pops") == 0)
    {
        patchPops(mod);
    }

    if(g_previous)
    {
        g_previous(mod);
        return;
    }
}

struct FunctionHook
{
    unsigned int nid;
    void *fp;
};

static const char *getFileBasename(const char *path)
{
    const char *p;

    if(path == NULL)
    {
        return NULL;
    }

    p = strrchr(path, '/');

    if(p == NULL)
    {
        p = path;
    }
    else
    {
        p++;
    }

    return p;
}

static inline int isEbootPBP(const char *path)
{
    const char *p;

    p = getFileBasename(path);

    if(p != NULL && 0 == strcmp(p, "EBOOT.PBP"))
    {
        return 1;
    }

    return 0;
}

static int checkFileDecrypted(const char *filename)
{
    SceUID fd = -1;
    u32 k1;
    int result = 0, ret;
    u8 p[16 + 64], *buf;
    u32 *magic;

    buf = (u8*)((((u32)p) & ~(64-1)) + 64);

    if(!g_isCustomPBP && isEbootPBP(filename))
    {
        return 0;
    }

    k1 = pspSdkSetK1(0);

    fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);

    if(fd < 0)
    {
        goto exit;
    }

    ret = sceIoRead(fd, buf, 16);

    if(ret != 16)
    {
        goto exit;
    }

    magic = (u32*)buf;

    // PGD
    if(*magic == 0x44475000)
    {
        goto exit;
    }

    result = 1;

exit:
    if(fd >= 0)
    {
        sceIoClose(fd);
    }

    pspSdkSetK1(k1);

    return result;
}

static inline int isDocumentPath(const char *path)
{
    const char *p;

    p = getFileBasename(path);
    
    if(p != NULL && 0 == strcmp(p, "DOCUMENT.DAT"))
    {
        return 1;
    }

    return 0;
}

static int sceIoOpenPlain(const char *file, int flag, int mode)
{
    int ret;

    if(flag == 0x40000001 && checkFileDecrypted(file))
    {
        #ifdef DEBUG
        printk("%s: removed PGD open flag\r\n", __func__);
        #endif
        ret = sceIoOpen(file, flag & ~0x40000000, mode);

        if(ret >= 0 && isDocumentPath(file))
        {
            g_plain_doc_fd = ret;
        }
    }
    else
    {
        ret = sceIoOpen(file, flag, mode);
    }

    return ret;
}

static int myIoOpen(const char *file, int flag, int mode)
{
    int ret;

    if(g_keysBinFound || g_isCustomPBP)
    {
        if(strstr(file, PGD_ID))
        {
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
            ret = RIF_MAGIC_FD;
        } else if (0 == strcmp(file, ACT_DAT))
        {
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
            ret = ACT_DAT_FD;
        } 
        else
        {
            ret = sceIoOpenPlain(file, flag, mode);
        }        
    }
    else
    {
        ret = sceIoOpenPlain(file, flag, mode);
    }

    #ifdef DEBUG
    printk("%s: %s 0x%08X -> 0x%08X\r\n", __func__, file, flag, ret);
    #endif
    return ret;
}

static int myIoIoctl(SceUID fd, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen)
{
    int ret;

    #ifdef DEBUG
    if(cmd == 0x04100001)
    {
        printk("%s: setting PGD key\r\n", __func__);
    }

    if(cmd == 0x04100002)
    {
        printk("%s: setting PGD offset: 0x%08X\r\n", __func__, *(uint*)indata);
    }
    #endif

    if (g_isCustomPBP || (g_plain_doc_fd >= 0 && g_plain_doc_fd == fd))
    {
        if (cmd == 0x04100001)
        {
            ret = 0;
            #ifdef DEBUG
            printk("%s: [FAKE] 0x%08X 0x%08X -> 0x%08X\r\n", __func__, fd, cmd, ret);
            #endif
            goto exit;
        }

        if (cmd == 0x04100002)
        {
            ret = sceIoLseek32(fd, *(u32*)indata, PSP_SEEK_SET);

            #ifdef DEBUG
            if(ret < 0)
            {
                printk("%s: sceIoLseek32 -> 0x%08X\r\n", __func__, ret);
            }
            #endif

            ret = 0;
            
            #ifdef DEBUG
            printk("%s: [FAKE] 0x%08X 0x%08X -> 0x%08X\r\n", __func__, fd, cmd, ret);
            #endif
            goto exit;
        }
    }

    ret = sceIoIoctl(fd, cmd, indata, inlen, outdata, outlen);

exit:
    #ifdef DEBUG
    printk("%s: 0x%08X -> 0x%08X\r\n", __func__, fd, ret);
    #endif
    return ret;
}

static int myIoGetstat(const char *path, SceIoStat *stat)
{
    int ret;

    if(g_keysBinFound || g_isCustomPBP)
    {
        if(strstr(path, PGD_ID))
        {
            stat->st_mode = 0x21FF;
            stat->st_attr = 0x20;
            stat->st_size = 152;
            ret = 0;
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
        } else if (0 == strcmp(path, ACT_DAT))
        {
            stat->st_mode = 0x21FF;
            stat->st_attr = 0x20;
            stat->st_size = 4152;
            ret = 0;
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
        } 
        else
        {
            ret = sceIoGetstat(path, stat);
        }
    } 
    else
    {
        ret = sceIoGetstat(path, stat);
    }
    #ifdef DEBUG
    printk("%s: %s -> 0x%08X\r\n", __func__, path, ret);
    #endif
    return ret;
}

static int myIoRead(int fd, unsigned char *buf, int size)
{
    int ret;
    u32 pos;
    u32 k1;

    UNUSED(pos);
    k1 = pspSdkSetK1(0);

    if(fd != RIF_MAGIC_FD && fd != ACT_DAT_FD)
    {
        pos = sceIoLseek32(fd, 0, SEEK_CUR);
    } 
    else
    {
        pos = 0;
    }
    
    if(g_keysBinFound|| g_isCustomPBP)
    {
        if(fd == RIF_MAGIC_FD)
        {
            size = 152;
            #ifdef DEBUG
            printk("%s: fake rif content %d\r\n", __func__, size);
            #endif
            memset(buf, 0, size);
            strcpy((char*)(buf+0x10), PGD_ID);
            ret = size;
            goto exit;
        } else if (fd == ACT_DAT_FD)
        {
            #ifdef DEBUG
            printk("%s: fake act.dat content %d\r\n", __func__, size);
            #endif
            memset(buf, 0, size);
            ret = size;
            goto exit;
        }
    }
    
    ret = sceIoRead(fd, buf, size);

    if(ret != size)
    {
        goto exit;
    }

    if (size == 4)
    {
        u32 magic = 0x464C457F; // ~ELF

        if(0 == memcmp(buf, &magic, sizeof(magic)))
        {
            magic = 0x5053507E; // ~PSP
            memcpy(buf, &magic, sizeof(magic));
            #ifdef DEBUG
            printk("%s: patch ~ELF -> ~PSP\r\n", __func__);
            #endif
        }

        ret = size;
        goto exit;
    }
    
    if(size == sizeof(g_icon_png))
    {
        u32 png_signature = 0x474E5089;

        if(g_icon0Status == ICON0_MISSING || ((g_icon0Status == ICON0_CORRUPTED) && 0 == memcmp(buf, &png_signature, 4)))
        {
            #ifdef DEBUG
            printk("%s: fakes a PNG for icon0\r\n", __func__);
            #endif
            memcpy(buf, g_icon_png, size);

            ret = size;
            goto exit;
        }
    }

    if (g_isCustomPBP && size >= 0x420 && buf[0x41B] == 0x27 &&
            buf[0x41C] == 0x19 &&
            buf[0x41D] == 0x22 &&
            buf[0x41E] == 0x41 &&
            buf[0x41A] == buf[0x41F])
    {
        buf[0x41B] = 0x55;
        #ifdef DEBUG
        printk("%s: unknown patch loc_6c\r\n", __func__);
        #endif
    }

exit:
    pspSdkSetK1(k1);
    #ifdef DEBUG
    printk("%s: fd=0x%08X pos=0x%08X size=%d -> 0x%08X\r\n", __func__, (uint)fd, (uint)pos, (int)size, ret);
    #endif
    return ret;
}

static int myIoReadAsync(int fd, unsigned char *buf, int size)
{
    int ret;
    unsigned int pos;
    unsigned int k1;

    UNUSED(pos);
    k1 = pspSdkSetK1(0);
    pos = sceIoLseek32(fd, 0, SEEK_CUR);
    pspSdkSetK1(k1);
    ret = sceIoReadAsync(fd, buf, size);
    printk("%s: 0x%08X 0x%08X 0x%08X -> 0x%08X\r\n", __func__, (uint)fd, (uint)pos, size, ret);

    return ret;
}

static SceOff myIoLseek(SceUID fd, SceOff offset, int whence)
{
    SceOff ret;
    u32 k1;

    k1 = pspSdkSetK1(0);

    if(g_keysBinFound || g_isCustomPBP)
    {
        if (fd == RIF_MAGIC_FD)
        {
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
            ret = 0;
        } else if (fd == ACT_DAT_FD)
        {
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
            ret = 0;
        } 
        else
        {
            ret = sceIoLseek(fd, offset, whence);
        }
    } 
    else
    {
        ret = sceIoLseek(fd, offset, whence);
    }

    pspSdkSetK1(k1);
    #ifdef DEBUG
    printk("%s: 0x%08X 0x%08X 0x%08X -> 0x%08X\r\n", __func__, (uint)fd, (uint)offset, (uint)whence, (int)ret);
    #endif
    return ret;
}

static int myIoClose(SceUID fd)
{
    int ret;
    u32 k1;

    k1 = pspSdkSetK1(0);

    if(g_keysBinFound || g_isCustomPBP)
    {
        if (fd == RIF_MAGIC_FD)
        {
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
            ret = 0;
        } else if (fd == ACT_DAT_FD)
        {
            #ifdef DEBUG
            printk("%s: [FAKE]\r\n", __func__);
            #endif
            ret = 0;
        } 
        else
        {
            ret = sceIoClose(fd);
        }
    } 
    else
    {
        ret = sceIoClose(fd);
    }

    if(g_plain_doc_fd == fd && ret == 0)
    {
        g_plain_doc_fd = -1;
    }

    pspSdkSetK1(k1);
    #ifdef DEBUG
    printk("%s: 0x%08X -> 0x%08X\r\n", __func__, fd, ret);
    #endif
    return ret;
}

static struct FunctionHook g_ioHooks[] = {
    { 0x109F50BC, &myIoOpen, },
    { 0x27EB27B8, &myIoLseek, },
    { 0x63632449, &myIoIoctl, },
    { 0x6A638D83, &myIoRead, },
    { 0xA0B5A7C2, &myIoReadAsync, },
    { 0xACE946E8, &myIoGetstat, },
    { 0x810C4BC3, &myIoClose, },
};

static struct FunctionHook g_amctrlHooks[] = {
    { 0x1CCB66D2, NULL},
    { 0x0785C974, NULL},
    { 0x9951C50F, NULL},
    { 0x525B8218, NULL},
    { 0x58163FBE, NULL},
    { 0xEF95A213, NULL},
    { 0xF5186D8E, NULL},
};

static int (*sceNpDrmGetVersionKey)(unsigned char * key, unsigned char * act, unsigned char * rif, unsigned int flags);
static int _sceNpDrmGetVersionKey(unsigned char * key, unsigned char * act, unsigned char * rif, unsigned int flags)
{
    char keypath[128];
    int result;
   
    result = (*sceNpDrmGetVersionKey)(key, act, rif, flags);

    if (g_isCustomPBP)
    {
        #ifdef DEBUG
        printk("%s: -> 0x%08X\r\n", __func__, result);
        #endif
        result = 0;

        if (g_keysBinFound)
        {
            memcpy(key, g_keys, sizeof(g_keys));
        }
        #ifdef DEBUG
        printk("%s:[FAKE] -> 0x%08X\r\n", __func__, result);
        #endif
    }
    else
    {
        getKeysBinPath(keypath, sizeof(keypath));

        if (result == 0)
        {
            int ret;

            UNUSED(ret);
            memcpy(g_keys, key, sizeof(g_keys));
            ret = saveKeysBin(keypath, g_keys, sizeof(g_keys));
            #ifdef DEBUG
            printk("%s: saveKeysBin -> %d\r\n", __func__, ret);
            #endif
        }
        else
        {
            if (g_keysBinFound)
            {
                memcpy(key, g_keys, sizeof(g_keys));
                result = 0;
            }
        }
    }
    
    return result;
}

static int (*scePspNpDrm_driver_9A34AC9F)(unsigned char *rif);
static int _scePspNpDrm_driver_9A34AC9F(unsigned char *rif)
{
    int result;

    result = (*scePspNpDrm_driver_9A34AC9F)(rif);
    #ifdef DEBUG
    printk("%s: 0x%08X -> 0x%08X\r\n", __func__, (uint)rif, result);
    #endif
    if (result != 0)
    {
        if (g_keysBinFound || g_isCustomPBP)
        {
            result = 0;
            #ifdef DEBUG
            printk("%s:[FAKE] -> 0x%08X\r\n", __func__, result);
            #endif
        }
    }

    return result;
}

static int (*_getRifPath)(const char *name, char *path) = NULL;
static int getRifPatch(char *name, char *path)
{
    int ret;

    if(g_keysBinFound || g_isCustomPBP) {
        strcpy(name, PGD_ID);
    }

    ret = (*_getRifPath)(name, path);
    #ifdef DEBUG
    printk("%s: %s %s -> 0x%08X\r\n", __func__, name, path, ret);
    #endif
    return ret;
}

void patchPopsMgr(void)
{
    SceModule2 *mod = (SceModule2*) sceKernelFindModuleByName("scePops_Manager");
    unsigned int text_addr = mod->text_addr;
    int i;
    
    sceNpDrmGetVersionKey = (void*)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_driver", 0x0F9547E6);
    scePspNpDrm_driver_9A34AC9F = (void*)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_driver", 0x9A34AC9F);
    hookImportByNID(mod, "scePspNpDrm_driver", 0x0F9547E6, _sceNpDrmGetVersionKey);
    hookImportByNID(mod, "scePspNpDrm_driver", 0x9A34AC9F, _scePspNpDrm_driver_9A34AC9F);

    for(i=0; i<NELEMS(g_ioHooks); ++i)
    {
        hookImportByNID(mod, "IoFileMgrForKernel", g_ioHooks[i].nid, g_ioHooks[i].fp);
    }

    if (g_isCustomPBP)
    {
        for(i=0; i<NELEMS(g_amctrlHooks); ++i)
        {
            hookImportByNID(mod, "sceAmctrl_driver", g_amctrlHooks[i].nid, g_amctrlHooks[i].fp);
        }
    }

    // patch popsman
    int patches = 3;
    for (u32 addr = text_addr; addr<text_addr+mod->text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x34C20016){
            _getRifPath = (void*)(addr-32); // found getRifPath
        }
        else if (data == JAL(_getRifPath)){
            _sw(JAL(&getRifPatch), addr); // redirect calls to getRifPath
            patches--;
        }
        else if (data == 0x0000000D){
            _sw(NOP, addr); // remove the check in scePopsManLoadModule that only allows loading module below the FW 3.XX
            patches--;
        }
    }

}

static unsigned int isCustomPBP(void)
{
    SceUID fd = -1;
    const char *filename;
    int result, ret;
    unsigned int psar_offset, pgd_offset, *magic;
    unsigned char p[40 + 64], *header;

    header = (unsigned char*)((((unsigned int)p) & ~(64-1)) + 64);
    filename = sceKernelInitFileName();
    result = 0;

    if(filename == NULL)
    {
        result = 0;
        goto exit;
    }

    fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);

    if(fd < 0)
    {
        #ifdef DEBUG
        printk("%s: sceIoOpen %s -> 0x%08X\r\n", __func__, filename, fd);
        #endif
        result = 0;
        goto exit;
    }

    ret = sceIoRead(fd, header, 40);

    if(ret != 40)
    {
        #ifdef DEBUG
        printk("%s: sceIoRead -> 0x%08X\r\n", __func__, ret);
        #endif
        result = 0;
        goto exit;
    }

    psar_offset = *(unsigned int*)(header+0x24);
    sceIoLseek32(fd, psar_offset, PSP_SEEK_SET);
    ret = sceIoRead(fd, header, 40);

    if(ret != 40)
    {
        #ifdef DEBUG
        printk("%s: sceIoRead -> 0x%08X\r\n", __func__, ret);
        #endif
        result = 0;
        goto exit;
    }

    pgd_offset = psar_offset;

    if(0 == memcmp(header, "PSTITLE", sizeof("PSTITLE")-1))
    {
        pgd_offset += 0x200;
    }
    else
    {
        pgd_offset += 0x400;
    }

    sceIoLseek32(fd, pgd_offset, PSP_SEEK_SET);
    ret = sceIoRead(fd, header, 4);

    if(ret != 4)
    {
        #ifdef DEBUG
        printk("%s: sceIoRead -> 0x%08X\r\n", __func__, ret);
        #endif
        result = 0;
        goto exit;
    }

    magic = (unsigned int*)header;

    // PGD offset
    if(*magic != 0x44475000)
    {
        #ifdef DEBUG
        printk("%s: custom pops found\r\n", __func__);
        #endif
        result = 1;
    }

exit:
    if(fd >= 0)
    {
        sceIoClose(fd);
    }

    return result;
}

static int (*sceMeAudio_67CD7972)(void *buf, int size);
int _sceMeAudio_67CD7972(void *buf, int size)
{
    int ret;
    unsigned int k1;

    k1 = pspSdkSetK1(0);
    ret = (*sceMeAudio_67CD7972)(buf, size);
    pspSdkSetK1(k1);
    #ifdef DEBUG
    printk("%s: 0x%08X -> 0x%08X\r\n", __func__, size, ret);
    #endif
    return ret;
}

static int (*_SysMemUserForUser_315AD3A0)(unsigned int fw_version);
static void setupPsxFwVersion(unsigned int fw_version)
{

    if (_SysMemUserForUser_315AD3A0 == NULL)
        _SysMemUserForUser_315AD3A0 = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x315AD3A0);

    if (_SysMemUserForUser_315AD3A0 != NULL)
    {
        _SysMemUserForUser_315AD3A0(fw_version);
    }
}

static int getIcon0Status(void)
{
    unsigned int icon0_offset = 0, icon0_size = 0;
    int result = ICON0_MISSING;
    SceUID fd = -1;;
    const char *filename;
    unsigned char p[40 + 64], *header;
    
    header = (unsigned char*)((((unsigned int)p) & ~(64-1)) + 64);
    filename = sceKernelInitFileName();

    if(filename == NULL)
    {
        return ICON0_MISSING;
    }
    
    fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);

    if(fd < 0)
    {
        return ICON0_MISSING;
    }
    
    sceIoRead(fd, header, 40);
    sceIoClose(fd);

    icon0_offset = *(unsigned int*)(header+12);
    icon0_size =  *(unsigned int*)(header+16) - icon0_offset;

    return (icon0_size)? ICON0_OK : ICON0_MISSING;
}

static int getKeysBinPath(char *keypath, unsigned int size)
{
    char *p;

    strncpy(keypath, sceKernelInitFileName(), size);
    keypath[size-1] = '\0';
    p = strrchr(keypath, '/');

    if(p == NULL)
    {
        return -1;
    }

    p[1] = '\0';

    if(strlen(keypath) > size - (sizeof("KEYS.BIN") - 1) - 1)
    {
        #ifdef DEBUG
        printk("popcorn: %s too long\r\n", keypath);
        _sw(0, 0);
        #endif
        return -1;
    }

    strcat(keypath, "KEYS.BIN");

    return 0;
}

static int loadKeysBin(const char *keypath, unsigned char *key, int size)
{
    SceUID keys; 
    int ret;

    keys = sceIoOpen(keypath, PSP_O_RDONLY, 0777);

    if (keys < 0)
    {
        #ifdef DEBUG
        printk("%s: sceIoOpen %s -> 0x%08X\r\n", __func__, keypath, keys);
        #endif
        return -1;
    }

    ret = sceIoRead(keys, key, size); 

    if (ret == size)
    {
        ret = 0;
    } 
    else
    {
        ret = -2;
    }

    sceIoClose(keys);

    return ret;
}

static int saveKeysBin(const char *keypath, unsigned char *key, int size)
{
    SceUID keys;
    int ret;

    keys = sceIoOpen(keypath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    if (keys < 0)
    {
        return -1;
    }

    ret = sceIoWrite(keys, key, size);

    if(ret == size)
    {
        ret = 0;
    } 
    else
    {
        ret = -2;
    }

    sceIoClose(keys);

    return ret;
}

void getKeys(void)
{
    char keypath[512];
    int ret;
    SceIoStat stat;

    getKeysBinPath(keypath, sizeof(keypath));
    ret = sceIoGetstat(keypath, &stat);
    g_keysBinFound = 0;

    if(ret == 0)
    {
        if(loadKeysBin(keypath, g_keys, sizeof(g_keys)) == 0)
        {
            g_keysBinFound = 1;
            #ifdef DEBUG
            printk("popcorn: keys.bin found\r\n");
            #endif
        }
    }
}

static int patchSyscallStub(void* func, void *addr)
{
    unsigned int syscall_num;

    syscall_num = sceKernelQuerySystemCall(func);

    if(syscall_num == (unsigned int)-1)
    {
        return -1;
    }

    _sw(0x03E00008, (unsigned int)addr);
    _sw(((syscall_num<<6)|12), (unsigned int)(addr+4));

    return 0;
}

int decompressData(unsigned int destSize, const unsigned char *src, unsigned char *dest)
{
    unsigned int k1;
    int ret;

    k1 = pspSdkSetK1(0);

    ret = sceKernelDeflateDecompress(dest, destSize, src, 0);
    #ifdef DEBUG
    printk("%s: 0x%08X 0x%08X 0x%08X -> 0x%08X\r\n", __func__, (uint)destSize, (uint)src, (uint)dest, ret);
    #endif
    if (ret >= 0)
    {
        ret = 0x92FF;
        #ifdef DEBUG
        printk("%s: [FAKE] -> 0x%08X\r\n", __func__, ret);
        #endif
    }

    pspSdkSetK1(k1);

    return ret;
}

static int patchDecompressData(void *stub_addr, void *patch_addr)
{
    int ret;

    ret = patchSyscallStub(decompressData, stub_addr);

    if (ret != 0) 
    {
        #ifdef DEBUG
        printk("%s: patchSyscallStub -> 0x%08X\r\n", __func__, ret);
        #endif
        return -1;
    }

    _sw(JAL(stub_addr), (unsigned int)patch_addr);

    return 0;
}

static void patchPops(SceModule2 *mod)
{
    unsigned int text_addr = mod->text_addr;
    void *stub_addr=NULL, *patch_addr=NULL;
    #ifdef DEBUG
    printk("%s: patching pops\r\n", __func__);
    #endif
    for (u32 addr = text_addr; addr<text_addr+mod->text_size; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x8E66000C)
            patch_addr = (void*)(addr+8);
        else if (data == 0x3C1D09BF)
            stub_addr = (void*)(U_EXTRACT_CALL(addr-16));
        else if (data == 0x00432823 && g_icon0Status != ICON0_OK)
            _sw(0x24050000 | (sizeof(g_icon_png) & 0xFFFF), addr); // patch icon0 size
        else if (data == 0x24050080 && _lw(addr+24) == 0x24030001)
            _sw(0x24020001, addr+8); // Patch Manual Name Check
        else if ((data == 0x14C00014 && _lw(addr + 4) == 0x24E2FFFF) ||
            (data == 0x14A00014 && _lw(addr + 4) == 0x24C2FFFF))
        {   // Fix index length (enable CDDA)
            _sh(0x1000, addr + 2);
            _sh(0, addr + 4);
        }
    }

    if(g_isCustomPBP){
        patchDecompressData(stub_addr, patch_addr);
    }
    
    // Prevent Permission Problems
    sceMeAudio_67CD7972 = (void*)sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x2AB4FE43);
    hookImportByNID(mod, "sceMeAudio", 0x2AB4FE43, _sceMeAudio_67CD7972);

    flushCache();
}

int module_start(SceSize args, void* argp)
{
    #ifdef DEBUG
    printk("popcorn: init_file = %s\r\n", sceKernelInitFileName());

    char g_DiscID[32];
    u16 paramType = 0;
    u32 paramLength = sizeof(g_DiscID);
    sctrlGetInitPARAM("DISC_ID", &paramType, &paramLength, g_DiscID);
    
    printk("pops disc id: %s\r\n", g_DiscID);
    #endif

    g_pspFwVersion = sceKernelDevkitVersion();
    
    getKeys();
    g_isCustomPBP = isCustomPBP();
    g_icon0Status = getIcon0Status();

    if(g_isCustomPBP)
    {
        setupPsxFwVersion(g_pspFwVersion);
    }
    
    g_previous = sctrlHENSetStartModuleHandler(&popcornSyspatch);
    patchPopsMgr();
    
    flushCache();
    
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    return 0;
}
