#include "rebootconfig.h"

enum {
	PATCH_ADD,
	PATCH_OVERWRITE,
	PATCH_OVERRIDE
};

typedef struct ModuleList
{
	int patch_type;
	char * before_path;
	char * add_path;
	u16 flag;
	u16 loadmode;
} ModuleList;

typedef struct BtcnfHeader
{
	int signature; // 0
	int devkit; // 4
	int unknown[2]; // 8
	int modestart; // 0x10
	int nmodes; // 0x14
	int unknown2[2]; // 0x18
	int modulestart; // 0x20
	int nmodules; // 0x24
	int unknown3[2]; // 0x28
	int modnamestart; // 0x30
	int modnameend; // 0x34
	int unknown4[2]; // 0x38
} __attribute__((packed)) BtcnfHeader;


typedef struct ModuleEntry
{
	u32 stroffset;//0
	int reserved;//4
	u16 flags;//8
	u8 loadmode;//0x0a
	u8 loadmode2;//0x0B
	int reserved2;//0x0C
	u8 hash[0x10];//0x10
} __attribute__((packed)) ModuleEntry;

typedef struct ModeEntry
{
	u16 maxsearch;
	u16 searchstart; //
	int modeflag;
	int mode2;
	int reserved[5];
} __attribute__((packed)) ModeEntry;

/*
ModuleList module_inferno[] = {
	{ PATCH_OVERRIDE	,"/kd/np9660.prx", "/kd/ark_inferno.prx",  0x42  , 0x8001 },
	{ -1		,NULL			, NULL					, 0  , 0}
};

ModuleList module_np9660[] = {
	{ PATCH_ADD	,"/kd/np9660.prx", "/kd/pulsar.prx"		, 0x42  , 0x8001 },
	{ -1		,NULL			, NULL					, 0  , 0}
};
*/

ModuleList module_boot[] = {
	{ PATCH_ADD ,"/kd/init.prx"	, "/kd/ark_systemctrl.prx"	, 0xEF , 0x8001 },
	//{ PATCH_OVERRIDE	,"/kd/np9660.prx", "/kd/ark_inferno.prx",  0x42  , 0x8001 },
	//{ PATCH_ADD	,"/kd/usersystemlib.prx", "/kd/vshctrl_02g.prx"		,   1  , 0x8001 },
	//{ PATCH_ADD	,"/kd/popsman.prx", "/kd/horoscope.prx"		,   2 | 0x40 , 0x8001 },
	//{ PATCH_ADD	,"/kd/popsman.prx", "/kd/idmanager.prx"		,   8  , 0x8001 },
	{ -1		,NULL			, NULL					, 0  , 0}
};


static ModuleList *Get_list(ModuleList module_path[], const char * path)
{
	int i = 0;

	while( module_path[i].before_path )
	{
		if( strcmp( path , module_path[i].before_path ) == 0)
		{
			module_path[i].before_path = "";
			return module_path + i ;
		}
		i++;
	}

	return NULL;
}

int btcnf_patch(void *a0 , int size , ModuleList patch_list[] , int before , int after)
{
	ModuleList *list_stock;
	int ret = size;//
	int i , j;
	int module_cnt;

	BtcnfHeader *header = a0;

	if( header->signature == 0x0F803001)
	{
		module_cnt = header->nmodules;
//		printf("module_cnt:0x%08X\n",module_cnt);
		if( module_cnt > 0)
		{

			ModuleEntry *module_offset = (ModuleEntry *)((u32)header + (u32)(header->modulestart));
			char* modname_start = (char *)((u32)header+ header->modnamestart);
		
			for(i=0; i< module_cnt;i++)
			{				
				ModuleEntry *sp = &(module_offset[i]);

				if( before ){
					if( sp->flags & before){
						sp->flags |= after;
					}else{
						sp->flags &= ~(after);
					}
				}

				if( (list_stock = Get_list( patch_list, modname_start + sp->stroffset ) ) != NULL)
				{
//					printf("Add %s\n", list_stock->add_path );

					if( list_stock->patch_type == PATCH_OVERWRITE )
					{
						memcpy( modname_start + sp->stroffset , list_stock->add_path , strlen( list_stock->add_path )+1 );
					}
					else// if( list_stock->patch_type == PATCH_OVERRIDE || list_stock->patch_type == PATCH_ADD )
					{	

						if( list_stock->patch_type == PATCH_ADD ){
						    //printf("Adding %s\n", list_stock->add_path);
						    
						    u32 name_start = sp->stroffset;
						    u32 move_to = sp->stroffset + strlen(list_stock->add_path)+1;
						    
						    int s = (header->modnameend)-(header->modnamestart+name_start)-1; //header->modnameend-name_start;
						    /*
						    printf("-> %s\n", modname_start+name_start);
						    printf("moving %d bytes\n", s);
						    printf("ret: %d\n", ret);
						    printf("name_start: %d\n", header->modnamestart+name_start);
						    printf("name_end: %d\n", header->modnameend);
						    printf("move to: %d\n", header->modnamestart+move_to);
						    printf("moving to: %d, %s\n", modname_start+move_to, header+move_to);
						    */
						    //memcpy_b( header+move_to , header+name_start , s);
						    memcpy_b(modname_start+move_to, modname_start+name_start, s);
						    memcpy( modname_start+name_start , list_stock->add_path , strlen( list_stock->add_path )+1);
						    
						    ret += strlen( list_stock->add_path ) + 1;
							/*
							printf("-> %s\n", modname_start+name_start);
							printf("-> %s\n", modname_start+move_to);
							printf("-> %s\n", modname_start+name_start+strlen(list_stock->add_path)+1);
							*/
							u32 sp_off = (u32)sp-(u32)header;
							//printf("**: %d\n", ret-sp_off);
							
							memcpy_b( &(module_offset[i+1]) , sp , ret/*header->modulestart - 32*i*/);
							//sp->stroffset += 32;
							//printf("Modname: %s\n", modname_start+sp->stroffset);
							
							ret += 32;
							header->nmodules ++;
							header->modnamestart += 0x20;
							header->modnameend += 0x20;

							module_cnt ++;

							int mode_cnt = header->nmodes;
							ModeEntry * mode_entyr = (ModeEntry *)( (u32)header + (u32)(header->modestart));
							for(j=0;j< mode_cnt;j++)
							{
								mode_entyr[j].maxsearch ++;
								mode_entyr[j].searchstart = 0;
							}
							/*
							for (j=0; j<i; j++){
							    module_offset[j].stroffset += 32;
							}
							*/
							for (j=i+1; j<module_cnt; j++){
							    //module_offset[j].stroffset += 32;
							    //printf("%s -> ", modname_start+module_offset[j].stroffset);
							    module_offset[j].stroffset += strlen(list_stock->add_path)+1;
							    //printf("%s\n", modname_start+module_offset[j].stroffset);
							}
						    header->modnameend += strlen( list_stock->add_path ) +1;
						    sp->flags= list_stock->flag;//flag
					        sp->loadmode=  list_stock->loadmode;
					        modname_start =(char *)((u32)header+ header->modnamestart);
						}
						//sp->stroffset = header->modnameend - header->modnamestart;			

						//memcpy( (char *)((u32)header + (u32)(header->modnameend)) , list_stock->add_path , strlen( list_stock->add_path )+1 );
						//strcpy((char *)((u32)header + (u32)(header->modnameend)) , list_stock->add_path);
					}
				}
			}
		}	
	}

	return ret;
}

int _UnpackBootConfig(char **p_buffer, int length)
{
	int result;
	int newsize;
	char *buffer;

	result = (*UnpackBootConfig)(*p_buffer, length);
	
	return result;
	
	buffer = (void*)BOOTCONFIG_TEMP_BUFFER;
	memcpy(buffer, *p_buffer, length);
	*p_buffer = buffer;
	
	newsize = btcnf_patch(buffer, result, module_boot, 0, 0);
	if (newsize>result) result = newsize;
	
	return result;
}
