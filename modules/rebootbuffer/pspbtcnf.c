#include "rebootconfig.h"

#define PATH_FLASH0 "flash0:/"
//#define PATH_USBDEVICE PATH_FLASH0 "kd/_usbdevice.prx"
#define PATH_SYSTEMCTRL PATH_FLASH0 "kd/ark_systemctrl.prx"
#define PATH_VSHCTRL PATH_FLASH0 "kd/ark_vshctrl.prx"
#define PATH_GALAXY PATH_FLASH0 "kd/ark_galaxy.prx"
#define PATH_STARGATE PATH_FLASH0 "kd/ark_stargate.prx"
// No march33 for ARK.
//#define PATH_MARCH33 PATH_FLASH0 "kd/_march33.prx"
#define PATH_INFERNO PATH_FLASH0 "kd/ark_inferno.prx"
#define PATH_POPCORN PATH_FLASH0 "kd/ark_popcorn.prx"
// No recovery mode
//#define PATH_SATELITE PATH_FLASH0 "vsh/module/_satelite.prx"
//#define PATH_RECOVERY PATH_FLASH0 "vsh/module/_recovery.prx"

// No fatms
//#define PATH_FATMS_HELPER PATH_FLASH0 "kd/_fatmshlp.prx"
//#define PATH_FATMS_371 PATH_FLASH0 "kd/_fatms371.prx"

typedef struct _btcnf_header
{
	unsigned int signature; // 0
	unsigned int devkit; // 4
	unsigned int unknown[2]; // 8
	unsigned int modestart; // 0x10
	int nmodes; // 0x14
	unsigned int unknown2[2]; // 0x18
	unsigned int modulestart; // 0x20
	int nmodules; // 0x24
	unsigned int unknown3[2]; // 0x28
	unsigned int modnamestart; // 0x30
	unsigned int modnameend; // 0x34
	unsigned int unknown4[2]; // 0x38
} _btcnf_header __attribute((packed));

typedef struct _btcnf_module
{
	unsigned int module_path; // 0x00
	unsigned int unk_04; //0x04
	unsigned int flags; //0x08
	unsigned int unk_C; //0x0C
	unsigned char hash[0x10]; //0x10
} _btcnf_module __attribute((packed));

enum {
	VSH_RUNLEVEL     =     0x01,
	GAME_RUNLEVEL    =     0x02,
	UPDATER_RUNLEVEL =     0x04,
	POPS_RUNLEVEL    =     0x08,
	APP_RUNLEVEL     =     0x20,
	UMDEMU_RUNLEVEL  =     0x40,
	MLNAPP_RUNLEVEL  =     0x80,
};

int _strlen(char * string)
{
	//length
	int length = 0;

	//count bytes
	for(; string[length]; length++);

	return length;
}

int _memcmp(char * buf1, char * buf2, int length)
{
	//result
	int result = -1;

	//compare bytes
	int pos = 0; for(; pos < length; pos++)
	{
		//store comparison
		result = buf1[pos] - buf2[pos];

		//stop comparison
		if(result) break;
	}

	return result;
}

int _strcmp(char * str1, char * str2)
{
	//result
	int result = -1;

	//valid length
	if(_strlen(str1) == _strlen(str2))
	{
		//compare bytes
		result = _memcmp(str1, str2, _strlen(str1));
	}

	return result;
}

void _memmove(char * to, char * from, unsigned int length)
{
	//result
	int result = 0;

	if (to > from) {
		//back buffer
		char * tob = to + length;
		char * fromb = from + length;

		//loop copy
		unsigned int pos = 0; for(; pos < length; pos++)
		{
			//copy byte
			*--tob = *--fromb;

			//increment result
			result++;
		}

		return result;
	}

	return _memcpy(to, from, length);
}

int _memcpy(char * to, char * from, unsigned int length)
{
	//result
	int result = 0;

	//loop copy
	unsigned int pos = 0; for(; pos < length; pos++)
	{
		//copy byte
		to[pos] = from[pos];

		//increment result
		result++;
	}

	return result;
}

int _strcpy(char * to, char * from)
{
	//result
	int result = 0;

	//copy bytes
	result = _memcpy(to, from, _strlen(from) + 1);

	return result;
}

int _memset(unsigned char * buffer, unsigned char value, unsigned int length)
{
	//result
	int result = 0;

	//loop set
	unsigned int pos = 0; for(; pos < length; pos++)
	{
		//set byte
		buffer[pos] = value;

		//increment result
		result++;
	}

	return result;
}

int patch_bootconf_vsh(char *buffer, int length)
{
	int newsize, result;

	result = length;
	newsize = AddPRX(buffer, "/kd/vshbridge.prx", PATH_VSHCTRL+sizeof(PATH_FLASH0)-2, VSH_RUNLEVEL );

	if (newsize > 0) result = newsize;

	return result;
}

int patch_bootconf_pops(char *buffer, int length)
{
	int newsize, result;

	result = length;
	newsize = AddPRX(buffer, "/kd/usersystemlib.prx", PATH_POPCORN+sizeof(PATH_FLASH0)-2, POPS_RUNLEVEL);

	if (newsize > 0) result = newsize;

	return result;
}

struct add_module {
	char *prxname;
	char *insertbefore;
	u32 flags;
};

struct del_module {
	char *prxname;
	u32 flags;
};

static struct add_module np9660_add_mods[] = {
	{ "/kd/mgr.prx", "/kd/amctrl.prx", GAME_RUNLEVEL },
	{ "/kd/npdrm.prx", "/kd/iofilemgr_dnas.prx", GAME_RUNLEVEL },
	{ PATH_GALAXY+sizeof(PATH_FLASH0)-2, "/kd/np9660.prx", UMDEMU_RUNLEVEL },
	{ PATH_GALAXY+sizeof(PATH_FLASH0)-2, "/kd/utility.prx", GAME_RUNLEVEL },
	{ "/kd/np9660.prx", "/kd/utility.prx", GAME_RUNLEVEL },
	{ "/kd/isofs.prx", "/kd/utility.prx", GAME_RUNLEVEL },
};

static struct del_module np9660_del_mods[] = {
	{ "/kd/mediaman.prx", GAME_RUNLEVEL },
	{ "/kd/ata.prx", GAME_RUNLEVEL },
	{ "/kd/umdman.prx", GAME_RUNLEVEL },
	{ "/kd/umdcache.prx", GAME_RUNLEVEL },
	{ "/kd/umd9660.prx", GAME_RUNLEVEL },
};

int patch_bootconf_np9660(char *buffer, int length)
{
	int newsize, result, ret;

	result = length;

	int i; for(i=0; i<NELEMS(np9660_del_mods); ++i) {
		RemovePrx(buffer, np9660_del_mods[i].prxname, np9660_del_mods[i].flags);
	}

	for(i=0; i<NELEMS(np9660_add_mods); ++i) {
		newsize = MovePrx(buffer, np9660_add_mods[i].insertbefore, np9660_add_mods[i].prxname, np9660_add_mods[i].flags);

		if (newsize > 0) result = newsize;
	}

	return result;
}

static struct add_module inferno_add_mods[] = {
	{ "/kd/mgr.prx", "/kd/amctrl.prx", GAME_RUNLEVEL },
	{ PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/utility.prx", GAME_RUNLEVEL },
	{ PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/isofs.prx", UMDEMU_RUNLEVEL },
	{ "/kd/isofs.prx", "/kd/utility.prx", GAME_RUNLEVEL },
};

static struct del_module inferno_del_mods[] = {
	{ "/kd/mediaman.prx", GAME_RUNLEVEL },
	{ "/kd/ata.prx", GAME_RUNLEVEL },
	{ "/kd/umdman.prx", GAME_RUNLEVEL },
	{ "/kd/umdcache.prx", GAME_RUNLEVEL },
	{ "/kd/umd9660.prx", GAME_RUNLEVEL },
	{ "/kd/np9660.prx", UMDEMU_RUNLEVEL },
};

int patch_bootconf_inferno(char *buffer, int length)
{
	int newsize, result, ret;

	result = length;

	int i; for(i=0; i<NELEMS(inferno_del_mods); ++i) {
		RemovePrx(buffer, inferno_del_mods[i].prxname, inferno_del_mods[i].flags);
	}

	for(i=0; i<NELEMS(inferno_add_mods); ++i) {
		newsize = MovePrx(buffer, inferno_add_mods[i].insertbefore, inferno_add_mods[i].prxname, inferno_add_mods[i].flags);

		if (newsize > 0) result = newsize;
	}

	return result;
}

static struct add_module vshumd_add_mods[] = {
	{ "/kd/isofs.prx", "/kd/utility.prx", VSH_RUNLEVEL },
	{ PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/chnnlsv.prx", VSH_RUNLEVEL },
};

static struct del_module vshumd_del_mods[] = {
	{ "/kd/mediaman.prx", VSH_RUNLEVEL },
	{ "/kd/ata.prx", VSH_RUNLEVEL },
	{ "/kd/umdman.prx", VSH_RUNLEVEL },
	{ "/kd/umd9660.prx", VSH_RUNLEVEL },
};

int patch_bootconf_vshumd(char *buffer, int length)
{
	int newsize, result, ret;

	result = length;

	int i; for(i=0; i<NELEMS(vshumd_del_mods); ++i) {
		RemovePrx(buffer, vshumd_del_mods[i].prxname, vshumd_del_mods[i].flags);
	}
	
	for(i=0; i<NELEMS(vshumd_add_mods); ++i) {
		newsize = MovePrx(buffer, vshumd_add_mods[i].insertbefore, vshumd_add_mods[i].prxname, vshumd_add_mods[i].flags);

		if (newsize > 0) result = newsize;
	}

	return result;
}

static struct add_module updaterumd_add_mods[] = {
	{ "/kd/isofs.prx", "/kd/utility.prx", UPDATER_RUNLEVEL },
	{ PATH_INFERNO+sizeof(PATH_FLASH0)-2, "/kd/chnnlsv.prx", UPDATER_RUNLEVEL },
};

static struct del_module updaterumd_del_mods[] = {
	{ "/kd/mediaman.prx", UPDATER_RUNLEVEL },
	{ "/kd/ata.prx", UPDATER_RUNLEVEL },
	{ "/kd/umdman.prx", UPDATER_RUNLEVEL },
	{ "/kd/umd9660.prx", UPDATER_RUNLEVEL },
};

int patch_bootconf_updaterumd(char *buffer, int length)
{
	int newsize, result, ret;

	result = length;

	int i; for(i=0; i<NELEMS(updaterumd_del_mods); ++i) {
		RemovePrx(buffer, updaterumd_del_mods[i].prxname, updaterumd_del_mods[i].flags);
	}
	
	for(i=0; i<NELEMS(updaterumd_add_mods); ++i) {
		newsize = MovePrx(buffer, updaterumd_add_mods[i].insertbefore, updaterumd_add_mods[i].prxname, updaterumd_add_mods[i].flags);

		if (newsize > 0) result = newsize;
	}

	return result;
}

int _UnpackBootConfig(char **p_buffer, int length)
{
	int result;
	int newsize;
	char *buffer;

	result = (*UnpackBootConfig)(*p_buffer, length);
	buffer = (void*)BOOTCONFIG_TEMP_BUFFER;
	_memcpy(buffer, *p_buffer, length);
	*p_buffer = buffer;
	
	newsize = AddPRX(buffer, "/kd/init.prx", PATH_SYSTEMCTRL+sizeof(PATH_FLASH0)-2, 0x000000EF);
	if (newsize > 0) result = newsize;
	
	newsize = patch_bootconf_inferno(buffer, length);
	if (newsize > 0) result = newsize;
    
    /*
	if (SearchPrx(buffer, "/vsh/module/vshmain.prx") >= 0) {
		newsize = patch_bootconf_vsh(buffer, length);
		if (newsize > 0) result = newsize;
	}
    
	newsize = patch_bootconf_pops(buffer, length);
	if (newsize > 0) result = newsize;

	newsize = AddPRX(buffer, "/kd/me_wrapper.prx", PATH_STARGATE+sizeof(PATH_FLASH0)-2, GAME_RUNLEVEL | UMDEMU_RUNLEVEL);
	if (newsize > 0) result = newsize;
	
	RebootBufferConfiguration * conf = (RebootBufferConfiguration*)REBOOTEX_CONFIG;
	switch(conf->iso_mode) {
		case MODE_NP9660:
			newsize = patch_bootconf_np9660(buffer, length);
			if (newsize > 0) result = newsize;
			break;
		case MODE_VSHUMD:
			newsize = patch_bootconf_vshumd(buffer, length);
			if (newsize > 0) result = newsize;
			break;
		case MODE_UPDATERUMD:
			newsize = patch_bootconf_updaterumd(buffer, length);
			if (newsize > 0) result = newsize;
			break;
		case MODE_INFERNO:
		default:
			newsize = patch_bootconf_inferno(buffer, length);
			if (newsize > 0) result = newsize;
			break;
	}
    */
	return result;
}

/*
int _UnpackBootConfig(char **p_buffer, int length)
{
	int result;
	int newsize;
	char *buffer;

	result = (*UnpackBootConfig)(*p_buffer, length);
	buffer = (void*)BOOTCONFIG_TEMP_BUFFER;
	_memcpy(buffer, *p_buffer, length);
	*p_buffer = buffer;

	if(ofw_mode) {
		goto exit;
	}

	newsize = AddPRX(buffer, "/kd/init.prx", PATH_SYSTEMCTRL+sizeof(PATH_FLASH0)-2, 0x000000EF);

	if (newsize > 0) result = newsize;

	if (SearchPrx(buffer, "/vsh/module/vshmain.prx") >= 0) {
		newsize = patch_bootconf_vsh(buffer, length);

		if (newsize > 0) result = newsize;
	}

	newsize = patch_bootconf_pops(buffer, length);

	if (newsize > 0) result = newsize;

	newsize = AddPRX(buffer, "/kd/me_wrapper.prx", PATH_STARGATE+sizeof(PATH_FLASH0)-2, GAME_RUNLEVEL | UMDEMU_RUNLEVEL);

	if (newsize > 0) result = newsize;
	
	switch(iso_mode) {
		case NP9660_MODE:
			newsize = patch_bootconf_np9660(buffer, length);

			if (newsize > 0) result = newsize;
			break;
		case MARCH33_MODE:
			newsize = patch_bootconf_march33(buffer, length);

			if (newsize > 0) result = newsize;
			break;
		case INFERNO_MODE:
			newsize = patch_bootconf_inferno(buffer, length);

			if (newsize > 0) result = newsize;
			break;
		case VSHUMD_MODE:
			newsize = patch_bootconf_vshumd(buffer, length);

			if (newsize > 0) result = newsize;
			break;
		case UPDATERUMD_MODE:
			newsize = patch_bootconf_updaterumd(buffer, length);

			if (newsize > 0) result = newsize;
			break;
		case NORMAL_MODE:
		default:
			break;
	}

	//reboot variable set
	if(loadrebootmodulebefore)
	{
		//add reboot prx entry
		newsize = AddPRX(buffer, loadrebootmodulebefore, "/rtm.prx", rebootmoduleflags);

		if(newsize > 0) result = newsize;
	}

	if(!recovery_mode && is_fatms371())
	{
		newsize = patch_bootconf_fatms371(buffer, length);

		if (newsize > 0) result = newsize;
	}

exit:
	if((!recovery_mode || ofw_mode) && is_permanent_mode()) {
		RenameModule(buffer, VSHMAIN + sizeof(PATH_FLASH0) - 2, VSHORIG + sizeof(PATH_FLASH0) - 2);
	}

	return result;
}
*/

int SearchPrx(char *buffer, const char *modname)
{
	//cast header
	_btcnf_header * header = (_btcnf_header *)buffer;

	if(header->signature != BTCNF_MAGIC) {
		return -1;
	}

	if(header->nmodules <= 0) {
		return -2;
	}

	if(header->nmodes <= 0) {
		return -3;
	}

	//cast module list
	_btcnf_module * module = (_btcnf_module *)(buffer + header->modulestart);

	//iterate modules
	int modnum = 0; for(; modnum < header->nmodules; modnum++)
	{
		//found module name
		if(_strcmp(buffer + header->modnamestart + module[modnum].module_path, modname) == 0)
		{
			//stop search
			break;
		}
	}

	//found module
	if(modnum >= header->nmodules) {
		return -4;
	}

	return modnum;
}

int AddPRXNoCopyName(char * buffer, char * insertbefore, int prxname_offset, u32 flags)
{
	int modnum;

	modnum = SearchPrx(buffer, insertbefore);

	if (modnum < 0) {
		return modnum;
	}

	_btcnf_header * header = (_btcnf_header *)buffer;

	//cast module list
	_btcnf_module * module = (_btcnf_module *)(buffer + header->modulestart);

	//add custom module
	_btcnf_module newmod; _memset(&newmod, 0, sizeof(newmod));

	newmod.module_path = prxname_offset - header->modnamestart;

	if(flags >= 0x0000FFFF) {
		newmod.flags = flags;
	} else {
		newmod.flags = 0x80010000 | (flags & 0x0000FFFF);
	}

	_memmove(&module[modnum + 1], &module[modnum + 0], buffer + header->modnameend - (unsigned int)&module[modnum + 0]);
	_memcpy(&module[modnum + 0], &newmod, sizeof(newmod));
	header->nmodules++;
	header->modnamestart += sizeof(newmod);
	header->modnameend += sizeof(newmod);

	//make mode include our module
	int modenum = 0; for(; modenum < header->nmodes; modenum++)
	{
		//increase module range
		*(unsigned short *)(buffer + header->modestart + modenum * 32) += 1;
	}

	return header->modnameend;
}

int AddPRX(char * buffer, char * insertbefore, char * prxname, u32 flags)
{
	int modnum;

	modnum = SearchPrx(buffer, prxname);

	if (modnum >= 0) {
		_btcnf_header * header = (_btcnf_header *)buffer;
		_btcnf_module * module = (_btcnf_module *)(buffer + header->modulestart);

		return AddPRXNoCopyName(buffer, insertbefore, header->modnamestart + module[modnum].module_path, flags);
	}

	modnum = SearchPrx(buffer, insertbefore);

	if (modnum < 0) {
		return modnum;
	}

	_btcnf_header * header = (_btcnf_header *)buffer;

	header->modnameend += _strcpy(buffer + header->modnameend, prxname);

	return AddPRXNoCopyName(buffer, insertbefore, header->modnameend - _strlen(prxname) - 1, flags);
}

void RemovePrx(char *buffer, const char *prxname, u32 flags)
{
	u32 old_flags;
	int ret;

	ret = GetPrxFlag(buffer, prxname, &old_flags);

	if (ret < 0)
		return ret;

	old_flags &= 0x0000FFFF;
	flags &= 0x0000FFFF;

	if (old_flags & flags) {
		// rewrite the flags to remove the modules from runlevels indicated by flags
		old_flags = old_flags & (~flags);
	}

	ModifyPrxFlag(buffer, prxname, 0x80010000 | (old_flags & 0x0000FFFF));
}

int MovePrx(char * buffer, char * insertbefore, const char * prxname, u32 flags)
{
	RemovePrx(buffer, prxname, flags);

	return AddPRX(buffer, insertbefore, prxname, flags);
}

// Note flags is 32-bits!
void ModifyPrxFlag(char *buffer, const char* modname, u32 flags)
{
	int modnum;

	modnum = SearchPrx(buffer, modname);

	if (modnum < 0) {
		return modnum;
	}

	_btcnf_header * header = (_btcnf_header *)buffer;
	
	//cast module list
	_btcnf_module * module = (_btcnf_module *)(buffer + header->modulestart);

	module[modnum].flags = flags;
}

// Note flags is 32-bits!
int GetPrxFlag(char *buffer, const char* modname, u32 *flags)
{
	int modnum;

	modnum = SearchPrx(buffer, modname);

	if (modnum < 0) {
		return modnum;
	}

	_btcnf_header * header = (_btcnf_header *)buffer;
	
	//cast module list
	_btcnf_module * module = (_btcnf_module *)(buffer + header->modulestart);

	*flags = module[modnum].flags;

	return 0;
}

// Note: new_mod_name cannot have longer filename than mod_name 
int RenameModule(void *buffer, char *mod_name, char *new_mod_name)
{
	int modnum;

	modnum = SearchPrx(buffer, mod_name);

	if (modnum < 0) {
		return modnum;
	}

	_btcnf_header * header = (_btcnf_header *)buffer;
	
	//cast module list
	_btcnf_module * module = (_btcnf_module *)(buffer + header->modulestart);

	_strcpy((char*)(buffer + header->modnamestart + module[modnum].module_path), new_mod_name);

	return 0;
}
