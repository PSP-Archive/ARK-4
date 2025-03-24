#include "registry.h"

int vctrlGetRegistryValue(const char *dir, const char *name, u32 *val)
{
    int ret = 0;
    struct RegParam reg;
    REGHANDLE h;
    int k1 = pspSdkSetK1(0);

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
        	REGHANDLE hk;
        	unsigned int type, size;

        	if(!sceRegGetKeyInfo(hd, name, &hk, &type, &size))
        	{
        		if(!sceRegGetKeyValue(hd, hk, val, 4))
        		{
        			ret = 1;
        			sceRegFlushCategory(hd);
        		}
        	}
        	sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }

    pspSdkSetK1(k1);
    return ret;
}

int vctrlSetRegistryValue(const char *dir, const char *name, u32 val)
{
    int ret = 0;
    struct RegParam reg;
    REGHANDLE h;
    int k1 = pspSdkSetK1(0);

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
        	if(!sceRegSetKeyValue(hd, name, &val, 4))
        	{
        		ret = 1;
        		sceRegFlushCategory(hd);
        	}
        	else
        	{
        		sceRegCreateKey(hd, name, REG_TYPE_INT, 4);
        		sceRegSetKeyValue(hd, name, &val, 4);
        		ret = 1;
        		sceRegFlushCategory(hd);
        	}
        	sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }

    pspSdkSetK1(k1);
    return ret;
}