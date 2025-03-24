#include <pspsdk.h>
#include <pspkernel.h>
#include <pspnand_driver.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pspipl_update.h>
#include <systemctrl.h>

PSP_MODULE_INFO("pspIplUpdate", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define PSP_NAND_PAGE_USER_SIZE        512
#define PSP_NAND_PAGE_SPARE_SIZE        16
#define PSP_NAND_PAGE_SPARE_SMALL_SIZE    (PSP_NAND_PAGE_SPARE_SIZE-4)
#define PSP_NAND_PAGE_TOTAL_SIZE        (PSP_NAND_PAGE_USER_SIZE+PSP_NAND_PAGE_SPARE_SIZE)
#define PSP_NAND_PAGES_PER_BLOCK        32

#define PSP_NAND_BLOCK_USER_SIZE        (PSP_NAND_PAGE_USER_SIZE*PSP_NAND_PAGES_PER_BLOCK)
#define PSP_NAND_BLOCK_SPARE_SIZE        (PSP_NAND_PAGE_SPARE_SIZE*PSP_NAND_PAGES_PER_BLOCK)	
#define PSP_NAND_BLOCK_SPARE_SMALL_SIZE    (PSP_NAND_PAGE_SPARE_SMALL_SIZE*PSP_NAND_PAGES_PER_BLOCK)
#define PSP_NAND_BLOCK_TOTAL_SIZE        (PSP_NAND_PAGE_TOTAL_SIZE*PSP_NAND_PAGES_PER_BLOCK)

#define PSP_NAND_TOTAL_BLOCKS        	2048
#define PSP_NAND_TOTAL_PAGES        	(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_PAGES_PER_BLOCK)
#define PSP_NAND_SIZE_USER        	(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_BLOCK_USER_SIZE)
#define PSP_NAND_SIZE_SPARE        	(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_BLOCK_SPARE_SIZE)
#define PSP_NAND_SIZE        		(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_BLOCK_TOTAL_SIZE)

#define PSP_IPL_MAX_DATA_BLOCKS        0x20
#define PSP_IPL_MAX_SIZE        	(PSP_NAND_BLOCK_USER_SIZE*PSP_IPL_MAX_DATA_BLOCKS)
#define PSP_IPL_SIGNATURE        	0x6DC64A38

u8  user[PSP_NAND_BLOCK_USER_SIZE], spare[PSP_NAND_BLOCK_SPARE_SMALL_SIZE];

int sceNandEraseIplBlockWithRetry(u32 ppn)
{
    if (ppn >= 0x600)
    {
        Kprintf("FATAL ERROR: ppn=0x%08X outside of IPL area.\n", ppn);
        return -1;
    }

    return sceNandEraseBlockWithRetry(ppn);
}

int sceNandWriteIplBlockWithVerify(u32 ppn, void *user, void *spare)
{
    if (ppn >= 0x600)
    {
        Kprintf("FATAL ERROR: ppn=0x%08X outside of IPL area.\n", ppn);
        return -1;
    }

    return sceNandWriteBlockWithVerify(ppn, user, spare);
}

int pspIplGetIpl(u8 *buf)
{
    u32 block, ppn;
    u16    blocktable[32];
    int i, res, nblocks, size;

    for (block = 4; block < 0x0C; block++)
    {
        ppn = block*PSP_NAND_PAGES_PER_BLOCK;		
        res = sceNandReadPagesRawAll(ppn, user, spare, 1);
        if (res < 0)
        {
        	//Printf("   Error reading page 0x%04X.\n", ppn);
        	return res;
        }

        if (spare[5] == 0xFF) // if good block 
        {
        	if (*(u32 *)&spare[8] == PSP_IPL_SIGNATURE)
        		break;
        }
    }

    if (block == 0x0C)
    {
        //Printf("   Cannot find IPL in nand!.\n");
        return -1;
    }

    for (nblocks = 0; nblocks < 32; nblocks++)
    {
        blocktable[nblocks] = *(u16 *)&user[nblocks*2];
        
        if (blocktable[nblocks] == 0)
        	break;		
    }

    size = 0;

    for (i = 0; i < nblocks; i++)
    {
        ppn = blocktable[i]*PSP_NAND_PAGES_PER_BLOCK;
        res = sceNandReadBlockWithRetry(ppn, buf, NULL);
        if (res < 0)
        {
        	//Printf("   Cannot read block ppn=0x%04.\n", ppn);
        	return res;
        }

        buf += PSP_NAND_BLOCK_USER_SIZE;
        
        size += PSP_NAND_BLOCK_USER_SIZE;
    }
    
    return size;
}

int pspIplClearIpl()
{
    u32 block, ppn;
    int res;

    for (block = 0; block < 0x30; block++)
    {
        ppn = block*PSP_NAND_PAGES_PER_BLOCK;

        res = sceNandEraseIplBlockWithRetry(ppn);
        if (res < 0)
        {
        	//sceNandDoMarkAsBadBlock(ppn);			
        }
    }

    return 0;
}

int pspIplSetIpl(u8 *buf, u32 size, u16 key)
{
    int i, res, nblocks, written;
    u32 block, ppn;
    u16 blocktable[32];
    u32 *p;
    
    nblocks = (size + (PSP_NAND_BLOCK_USER_SIZE-1)) / PSP_NAND_BLOCK_USER_SIZE;

    if (nblocks > PSP_IPL_MAX_DATA_BLOCKS)
    {
        //Printf("   IPL too big (%d).\n");
        return -1;
    }

    // Init spare data
    for (i = 0, p = (u32 *)spare; i < PSP_NAND_PAGES_PER_BLOCK; i++, p += (PSP_NAND_PAGE_SPARE_SMALL_SIZE/4))
    {
        p[0] = 0xFFFF | key << 16;
        p[1] = PSP_IPL_SIGNATURE;
        p[2] = 0xFFFFFFFF;
    }

    // Write data blocks
    block = 0x11;
    for (i = 0; i < nblocks; i++)
    {
        if (i == (nblocks-1))
        {
        	int mod = size % PSP_NAND_BLOCK_USER_SIZE;
        	
        	if (mod != 0)
        	{
        		memset(user, 0xFF, PSP_NAND_BLOCK_USER_SIZE);
        		memcpy(user, buf, mod);

        		buf = user;
        	}		
        }
        	
        while (1)
        {
        	ppn = block*PSP_NAND_PAGES_PER_BLOCK;
        	res = sceNandWriteIplBlockWithVerify(ppn, buf, spare);
        	if (res < 0)
        	{
        		res = sceNandEraseIplBlockWithRetry(ppn);
        		if (res < 0)
        		{	
        			//sceNandDoMarkAsBadBlock(ppn);					
        		}

        		block++;
        	}
        	else
        	{
        		blocktable[i] = block;
        		block++;
        		break;
        	}
        }

        buf += PSP_NAND_BLOCK_USER_SIZE;
    }

    // Write block table to all mirrors
    memset(user, 0, PSP_NAND_BLOCK_USER_SIZE);
    memcpy(user, blocktable, nblocks*2);
    memset(spare, 0xFF, PSP_NAND_BLOCK_SPARE_SMALL_SIZE);
    *(u32 *)&spare[4] = PSP_IPL_SIGNATURE;

    written = 0;
    
    for (block = 4; block < 0x0C; block++)
    {
        ppn = block*PSP_NAND_PAGES_PER_BLOCK;
        res = sceNandWriteIplBlockWithVerify(ppn, user, spare);
        if (res < 0)
        {
        	res = sceNandEraseIplBlockWithRetry(ppn);
        	if (res < 0)
        	{
        		//sceNandDoMarkAsBadBlock(ppn);				
        	}
        }
        else
        {
        	written = 1;
        }
    }

    if (!written)
    {
        //printf("   Cannot write IPL block table.\n");
        return -1;
    }

    return 0;
}

int pspIplUpdateGetIpl(u8 *buf)
{
    int k1 = pspSdkSetK1(0);
    sceNandLock(0);
    
    int res = pspIplGetIpl(buf);

    sceNandUnlock();
    pspSdkSetK1(k1);

    return res;
}

static void SetScrambleZero()
{
    int (* sceNandSetScramble)(int) = NULL;

    sceNandSetScramble = (void *)sctrlHENFindFunction("sceNAND_Updater_Driver", "sceNand_updater_driver", 0x0BEE8F36);
    if (!sceNandSetScramble)
    {
        sceNandSetScramble = (void *)sctrlHENFindFunction("sceLowIO_Driver", "sceNand_driver", 0x0BEE8F36);
    }

    if (sceNandSetScramble)
        sceNandSetScramble(0);
}

int pspIplUpdateClearIpl()
{
    int k1 = pspSdkSetK1(0);
    sceNandLock(1);
    
    SetScrambleZero();
    
    int res = pspIplClearIpl();

    sceNandUnlock();
    pspSdkSetK1(k1);

    return res;
}

int pspIplUpdateSetIpl(u8 *buf, u32 size, u16 key)
{
    int k1 = pspSdkSetK1(0);
    sceNandLock(1);
    
    SetScrambleZero();
    
    int res = pspIplSetIpl(buf, size, key);

    sceNandUnlock();
    pspSdkSetK1(k1);

    return res;
}

int module_start(SceSize args, void *argp)
{
    return 0;
}

int module_stop(void)
{
    return 0;
}
