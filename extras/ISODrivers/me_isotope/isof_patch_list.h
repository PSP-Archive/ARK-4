#ifndef __patch_list__
#define __patch_list__

struct Isof_list {
	u32 IsofPatchAddr1;
	u32 IsofPatchAddr2;
	u32 IsofPatchAddr3;
	u32 IsofPatchAddr4;
};


#if _PSP_FW_VERSION == 639
static const struct Isof_list isof_patch_list = {
	.IsofPatchAddr1 = 0x00004020,
	.IsofPatchAddr2 = 0x00004058,
	.IsofPatchAddr3 = 0x0000410C,
	.IsofPatchAddr4 = 0x000042E8,
};

#elif _PSP_FW_VERSION == 660
static const struct Isof_list isof_patch_list = {
	.IsofPatchAddr1 = 0x00003FEC,//0x00004020,
	.IsofPatchAddr2 = 0x00004024,//0x00004058,
	.IsofPatchAddr3 = 0x000040D8,//0x0000410C,
	.IsofPatchAddr4 = 0x000042B4,//0x000042E8,
};

#else
#error FW error
#endif

#endif