	.set noreorder
	
#include "pspstub.s"

	STUB_START "scePaf",0x00090011,0x00120005
	STUB_FUNC  0xAF067FA2,sce_paf_private_wcslen
	STUB_FUNC  0xA138A376,sce_paf_private_sprintf
	STUB_FUNC  0xB4652CFE,sce_paf_private_memcpy
	STUB_FUNC  0xD9E2D6E1,sce_paf_private_memset
	STUB_FUNC  0xD7DCB972,sce_paf_private_strlen
	STUB_FUNC  0x726DFBA9,sce_paf_private_strcpy
	STUB_FUNC  0x706ABBFF,sce_paf_private_strncpy
	STUB_FUNC  0x4CF09BA2,sce_paf_private_strcmp
	STUB_FUNC  0xE0B32AE8,sce_paf_private_strncmp
	STUB_FUNC  0xC3B2D310,sce_paf_private_strchr
	STUB_FUNC  0x22420CC7,sce_paf_private_strrchr
	STUB_FUNC  0x4CE9C8D7,sce_paf_private_strpbrk
	STUB_FUNC  0xA4B8A4E3,sce_paf_private_strtoul
	STUB_FUNC  0x5E909060,sce_paf_private_malloc
	STUB_FUNC  0xDB1612F1,sce_paf_private_free
	STUB_FUNC  0x3874A5F8,scePafGetText
	STUB_FUNC  0x50DCB767,PAF_Resource_GetPageNodeByID
	STUB_FUNC  0x64A31513,PAF_Resource_ResolveRefWString
	STUB_END

	STUB_START "vsh",0x00090011,0x00020005
	STUB_FUNC  0x6C582683,vshGetRegistryValue
	STUB_FUNC  0x858291A3,vshSetRegistryValue
	STUB_END

	STUB_START "sceVshCommonGui",0x00090011,0x00010005
	STUB_FUNC  0x8D157BBD,sceVshCommonGuiBottomDialog
	STUB_END
