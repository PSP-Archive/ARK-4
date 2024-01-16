#include <taihen.h>
#include <vitasdk.h>
#include <stdint.h>

void _start() __attribute__ ((weak, alias ("module_start")));

#define SCE_PSPEMU_CACHE_NONE 0x1
#define THUMB_SHUFFLE(x) ((((x) & 0xFFFF0000) >> 16) | (((x) & 0xFFFF) << 16))

SceUID sceIoOpenHook = -1;
tai_hook_ref_t sceIoOpenRef = NULL;

SceUID io_patch_path = -1;
SceUID io_patch_size = -1;
SceUID peripheral_patch = -1;

uint32_t nop_nop_opcode = 0xBF00BF00;
uint32_t mov_r2_r4_mov_r4_r2 = 0x46224614;
uint16_t b_send_resp = 0xEAFFFFEA;
uint32_t mips_move_a2_0 = 0x00003021;
uint32_t mips_nop = 0;

int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
int (* ScePspemuWritebackCache)(void *addr, int size);

void get_functions(uint32_t text_addr) {
	ScePspemuConvertAddress = (void *)text_addr + 0x6364 + 0x1;
	ScePspemuWritebackCache = (void *)text_addr + 0x6490 + 0x1;
}

static SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	
	// Virtual Kernel Exploit
	if (file != NULL && strstr(file, "__dokxploit__") != 0){
		uint32_t *m;
		
		// remove k1 checks in IoRead (lets you write into kram)
		m = (uint32_t *)ScePspemuConvertAddress(0x8805769c, SCE_PSPEMU_CACHE_NONE, 4);
		*m = mips_move_a2_0; // move $a2, 0
		ScePspemuWritebackCache(m, 4);

		// remove k1 checks in IoWrite (lets you read kram)
		m = (uint32_t *)ScePspemuConvertAddress(0x880577b0, SCE_PSPEMU_CACHE_NONE, 4);
		*m = mips_move_a2_0; // move $a2, 0
		ScePspemuWritebackCache(m, 4);

		// allow running any code as kernel (lets us pass function pointer as second argument of libctime)
		m = (uint32_t *)ScePspemuConvertAddress(0x88010044, SCE_PSPEMU_CACHE_NONE, 4);
		*m = mips_nop; // nop
		ScePspemuWritebackCache(m, 4);
		return 0;
	}
	
	return TAI_CONTINUE(SceUID, sceIoOpenRef, file, flags, mode);
}

uint32_t encode_bl(uint32_t patch_offset, uint32_t target_offset) {
  uint32_t displacement = target_offset - (patch_offset & ~0x1) - 4;
  uint32_t signbit = (displacement >> 31) & 0x1;
  uint32_t i1 = (displacement >> 23) & 0x1;
  uint32_t i2 = (displacement >> 22) & 0x1;
  uint32_t imm10 = (displacement >> 12) & 0x03FF;
  uint32_t imm11 = (displacement >> 1) & 0x07FF;
  uint32_t j1 = i1 ^ (signbit ^ 1);
  uint32_t j2 = i2 ^ (signbit ^ 1);
  uint32_t value = (signbit << 26) | (j1 << 13) | (j2 << 11) | (imm10 << 16) | imm11;
  value |= 0xF000D000;  // BL
  return THUMB_SHUFFLE(value);
}

int module_start(SceSize argc, const void *args) {
  tai_module_info_t info;
  info.size = sizeof(info);
  
  taiGetModuleInfo("ScePspemu", &info);
  
  SceKernelModuleInfo mod_info;
  mod_info.size = sizeof(SceKernelModuleInfo);
  int ret = sceKernelGetModuleInfo(info.modid, &mod_info);
  
  // Get PspEmu functions
  get_functions((uint32_t)mod_info.segments[0].vaddr);
  
  // allow opening any path
  io_patch_path = taiInjectData(info.modid, 0x00, 0x839C, &nop_nop_opcode, 0x4);
  
  // allow opening files of any size
  io_patch_size = taiInjectData(info.modid, 0x00, 0xA13C, &mov_r2_r4_mov_r4_r2, 0x4);
  
  // allow easy escalation of priviledges
  sceIoOpenHook = taiHookFunctionImport(&sceIoOpenRef, "ScePspemu", 0xCAE9ACE6, 0x6C60AC61, sceIoOpenPatched);
  
  // Adrenaline patches
  static SceUID uids[64];
  static int n_uids = 0;
  uint32_t text_addr, text_size;
  
  text_addr = (uint32_t)mod_info.segments[0].vaddr;
  text_size = (uint32_t)mod_info.segments[0].memsz;
  
  // Use different mode for ScePspemuRemotePocs
 
    //uint16_t movs_a1_E = 0x200E;
    //uids[n_uids++] = taiInjectData(info.modid, 0, 0x22734, &movs_a1_E, sizeof(movs_a1_E));


    // g_is_pops patches

    uint32_t movs_a4_1_nop_opcode = 0xBF002301;
    uint32_t movs_a1_0_nop_opcode = 0xBF002000;
    uint32_t movs_a1_1_nop_opcode = 0xBF002001;

    // Resume stuff. PROBABLY SHOULD DO POPS AND PSP MODE STUFF
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x42F0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown. Mode 4, 5
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x572E, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Set cache address for pops stuff
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x57C0, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Read savedata and menu info. Should be enabled, otherwise an error will occur
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x5BBA, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Get app state for pops
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x5C52, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    // Unknown
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x5E4A, &movs_a4_1_nop_opcode, sizeof(movs_a4_1_nop_opcode));

    ///////////////////////////

    // isPops patches

    // Peripheral

    // Use vibration
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x169F6, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for POPS mode
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16AEC, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0x80010089
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16B6C, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16B86, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Unknown check for PSP mode. If false return 0
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x16C3E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    ////////////////////

    // Init ScePspemuMenuWork
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x1825E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Read savedata and menu info
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x2121E, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // POPS Settings menu function
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x17B32, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    /////////////////////
    // Settings related. Screenshot is enabled/disabled here. Responsible for __sce_menuinfo saving
    uint32_t bl_is_pops_patched_opcode_1 = encode_bl(text_addr + 0x21022, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x21022, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    uint32_t bl_is_pops_patched_opcode_2 = encode_bl(text_addr + 0x2104C, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x2104C, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Switch between PSP mode settings and POPS mode settings
    uint32_t bl_is_pops_patched_opcode_3 = encode_bl(text_addr + 0x17BEA, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x17BEA, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // Draw dialog on PSP screen or POPS screen
    uint32_t bl_is_pops_patched_opcode_4 = encode_bl(text_addr + 0x183A4, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x183A4, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));

    // ctrlEmulation. If not patched, buttons assignment in ps1emu don't work
    uint32_t bl_is_pops_patched_opcode_5 = encode_bl(text_addr + 0x20710, text_addr + 0x20384);
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x20710, &movs_a1_1_nop_opcode, sizeof(movs_a1_1_nop_opcode));
    
    // Use available code memory at text_addr + 0x20384 (ScePspemuInitTitleSpecificInfo)
    // For custom function: isPopsPatched
    /*
    uint32_t isPopsPatched[2];
    isPopsPatched[0] = movs_a1_1_nop_opcode; // movs a1, 1
    isPopsPatched[1] = 0xBF004770; // bx lr
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x20384, isPopsPatched, sizeof(isPopsPatched));
    */

    // Fake vita mode for ctrlEmulation
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x2073C, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x2084E, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
    uids[n_uids++] = taiInjectData(info.modid, 0, 0x301DC, &movs_a1_0_nop_opcode, sizeof(movs_a1_0_nop_opcode));
  
  /*
static const uint8_t inj_0 = 0x0;
taiInjectData(info.modid, 0x00, 0x41d8, &inj_0, 0x1);
static const uint8_t inj_1 = 0x1;
taiInjectData(info.modid, 0x00, 0x5898, &inj_1, 0x1);
static const uint8_t inj_2 = 0x2;
taiInjectData(info.modid, 0x00, 0x8644, &inj_2, 0x1);
static const uint8_t inj_3 = 0x3;
taiInjectData(info.modid, 0x00, 0x89dc, &inj_3, 0x1);
static const uint8_t inj_4 = 0x4;
taiInjectData(info.modid, 0x00, 0x8b98, &inj_4, 0x1);
static const uint8_t inj_5 = 0x5;
taiInjectData(info.modid, 0x00, 0x8c60, &inj_5, 0x1);
static const uint8_t inj_6 = 0x6;
taiInjectData(info.modid, 0x00, 0x8d0c, &inj_6, 0x1);
static const uint8_t inj_7 = 0x7;
taiInjectData(info.modid, 0x00, 0x8daa, &inj_7, 0x1);
static const uint8_t inj_8 = 0x8;
taiInjectData(info.modid, 0x00, 0x8ed6, &inj_8, 0x1);
static const uint8_t inj_9 = 0x9;
taiInjectData(info.modid, 0x00, 0x956a, &inj_9, 0x1);
static const uint8_t inj_10 = 0xa;
taiInjectData(info.modid, 0x00, 0x9c56, &inj_10, 0x1);
static const uint8_t inj_11 = 0xb;
taiInjectData(info.modid, 0x00, 0x9e16, &inj_11, 0x1);
static const uint8_t inj_12 = 0xc;
taiInjectData(info.modid, 0x00, 0x9f02, &inj_12, 0x1);
static const uint8_t inj_13 = 0xd;
taiInjectData(info.modid, 0x00, 0xa14e, &inj_13, 0x1);
static const uint8_t inj_14 = 0xe;
taiInjectData(info.modid, 0x00, 0xa1ec, &inj_14, 0x1);
static const uint8_t inj_15 = 0xf;
taiInjectData(info.modid, 0x00, 0xa6e0, &inj_15, 0x1);
static const uint8_t inj_16 = 0x10;
taiInjectData(info.modid, 0x00, 0xb9ee, &inj_16, 0x1);
static const uint8_t inj_17 = 0x11;
taiInjectData(info.modid, 0x00, 0xcaf6, &inj_17, 0x1);
static const uint8_t inj_18 = 0x12;
taiInjectData(info.modid, 0x00, 0xccd0, &inj_18, 0x1);
static const uint8_t inj_19 = 0x13;
taiInjectData(info.modid, 0x00, 0xcf02, &inj_19, 0x1);
static const uint8_t inj_20 = 0x14;
taiInjectData(info.modid, 0x00, 0xd184, &inj_20, 0x1);
static const uint8_t inj_21 = 0x15;
taiInjectData(info.modid, 0x00, 0xd6e0, &inj_21, 0x1);
static const uint8_t inj_22 = 0x16;
taiInjectData(info.modid, 0x00, 0xdd56, &inj_22, 0x1);
static const uint8_t inj_23 = 0x17;
taiInjectData(info.modid, 0x00, 0xe154, &inj_23, 0x1);
static const uint8_t inj_24 = 0x18;
taiInjectData(info.modid, 0x00, 0xf386, &inj_24, 0x1);
static const uint8_t inj_25 = 0x19;
taiInjectData(info.modid, 0x00, 0xf3b4, &inj_25, 0x1);
static const uint8_t inj_26 = 0x1a;
taiInjectData(info.modid, 0x00, 0xf3d0, &inj_26, 0x1);
static const uint8_t inj_27 = 0x1b;
taiInjectData(info.modid, 0x00, 0xf5ca, &inj_27, 0x1);
static const uint8_t inj_28 = 0x1c;
taiInjectData(info.modid, 0x00, 0xf69e, &inj_28, 0x1);
static const uint8_t inj_29 = 0x1d;
taiInjectData(info.modid, 0x00, 0xf6da, &inj_29, 0x1);
static const uint8_t inj_30 = 0x1e;
taiInjectData(info.modid, 0x00, 0xf6f2, &inj_30, 0x1);
static const uint8_t inj_31 = 0x1f;
taiInjectData(info.modid, 0x00, 0xfad0, &inj_31, 0x1);
static const uint8_t inj_32 = 0x20;
taiInjectData(info.modid, 0x00, 0xfce0, &inj_32, 0x1);
static const uint8_t inj_33 = 0x21;
taiInjectData(info.modid, 0x00, 0xff2e, &inj_33, 0x1);
static const uint8_t inj_34 = 0x22;
taiInjectData(info.modid, 0x00, 0x10222, &inj_34, 0x1);
static const uint8_t inj_35 = 0x23;
taiInjectData(info.modid, 0x00, 0x10394, &inj_35, 0x1);
static const uint8_t inj_36 = 0x24;
taiInjectData(info.modid, 0x00, 0x10e74, &inj_36, 0x1);
static const uint8_t inj_37 = 0x25;
taiInjectData(info.modid, 0x00, 0x111de, &inj_37, 0x1);
static const uint8_t inj_38 = 0x26;
taiInjectData(info.modid, 0x00, 0x11644, &inj_38, 0x1);
static const uint8_t inj_39 = 0x27;
taiInjectData(info.modid, 0x00, 0x11684, &inj_39, 0x1);
static const uint8_t inj_40 = 0x28;
taiInjectData(info.modid, 0x00, 0x1182a, &inj_40, 0x1);
static const uint8_t inj_41 = 0x29;
taiInjectData(info.modid, 0x00, 0x11ca6, &inj_41, 0x1);
static const uint8_t inj_42 = 0x2a;
taiInjectData(info.modid, 0x00, 0x11d9e, &inj_42, 0x1);
static const uint8_t inj_43 = 0x2b;
taiInjectData(info.modid, 0x00, 0x11eba, &inj_43, 0x1);
static const uint8_t inj_44 = 0x2c;
taiInjectData(info.modid, 0x00, 0x11fea, &inj_44, 0x1);
static const uint8_t inj_45 = 0x2d;
taiInjectData(info.modid, 0x00, 0x122ac, &inj_45, 0x1);
static const uint8_t inj_46 = 0x2e;
taiInjectData(info.modid, 0x00, 0x12416, &inj_46, 0x1);
static const uint8_t inj_47 = 0x2f;
taiInjectData(info.modid, 0x00, 0x12c7e, &inj_47, 0x1);
static const uint8_t inj_48 = 0x30;
taiInjectData(info.modid, 0x00, 0x132d6, &inj_48, 0x1);
static const uint8_t inj_49 = 0x31;
taiInjectData(info.modid, 0x00, 0x15252, &inj_49, 0x1);
static const uint8_t inj_50 = 0x32;
taiInjectData(info.modid, 0x00, 0x1562e, &inj_50, 0x1);
static const uint8_t inj_51 = 0x33;
taiInjectData(info.modid, 0x00, 0x15786, &inj_51, 0x1);
static const uint8_t inj_52 = 0x34;
taiInjectData(info.modid, 0x00, 0x167f8, &inj_52, 0x1);
static const uint8_t inj_53 = 0x35;
taiInjectData(info.modid, 0x00, 0x169ec, &inj_53, 0x1);
static const uint8_t inj_54 = 0x36;
taiInjectData(info.modid, 0x00, 0x16f9c, &inj_54, 0x1);
static const uint8_t inj_55 = 0x37;
taiInjectData(info.modid, 0x00, 0x17014, &inj_55, 0x1);
static const uint8_t inj_56 = 0x38;
taiInjectData(info.modid, 0x00, 0x17036, &inj_56, 0x1);
static const uint8_t inj_57 = 0x39;
taiInjectData(info.modid, 0x00, 0x178e2, &inj_57, 0x1);
static const uint8_t inj_58 = 0x3a;
taiInjectData(info.modid, 0x00, 0x181ee, &inj_58, 0x1);
static const uint8_t inj_59 = 0x3b;
taiInjectData(info.modid, 0x00, 0x1ae6c, &inj_59, 0x1);
static const uint8_t inj_60 = 0x3c;
taiInjectData(info.modid, 0x00, 0x1b5a6, &inj_60, 0x1);
static const uint8_t inj_61 = 0x3d;
taiInjectData(info.modid, 0x00, 0x1b676, &inj_61, 0x1);
static const uint8_t inj_62 = 0x3e;
taiInjectData(info.modid, 0x00, 0x1b844, &inj_62, 0x1);
static const uint8_t inj_63 = 0x3f;
taiInjectData(info.modid, 0x00, 0x1e9ce, &inj_63, 0x1);
static const uint8_t inj_64 = 0x40;
taiInjectData(info.modid, 0x00, 0x1e9f0, &inj_64, 0x1);
static const uint8_t inj_65 = 0x41;
taiInjectData(info.modid, 0x00, 0x1ea88, &inj_65, 0x1);
static const uint8_t inj_66 = 0x42;
taiInjectData(info.modid, 0x00, 0x1ed46, &inj_66, 0x1);
static const uint8_t inj_67 = 0x43;
taiInjectData(info.modid, 0x00, 0x1ee98, &inj_67, 0x1);
static const uint8_t inj_68 = 0x44;
taiInjectData(info.modid, 0x00, 0x1ef16, &inj_68, 0x1);
static const uint8_t inj_69 = 0x45;
taiInjectData(info.modid, 0x00, 0x1efa0, &inj_69, 0x1);
static const uint8_t inj_70 = 0x46;
taiInjectData(info.modid, 0x00, 0x1eff2, &inj_70, 0x1);
static const uint8_t inj_71 = 0x47;
taiInjectData(info.modid, 0x00, 0x1f12e, &inj_71, 0x1);
static const uint8_t inj_72 = 0x48;
taiInjectData(info.modid, 0x00, 0x1f438, &inj_72, 0x1);
static const uint8_t inj_73 = 0x49;
taiInjectData(info.modid, 0x00, 0x1f442, &inj_73, 0x1);
static const uint8_t inj_74 = 0x4a;
taiInjectData(info.modid, 0x00, 0x1f532, &inj_74, 0x1);
static const uint8_t inj_75 = 0x4b;
taiInjectData(info.modid, 0x00, 0x1f6aa, &inj_75, 0x1);
static const uint8_t inj_76 = 0x4c;
taiInjectData(info.modid, 0x00, 0x1f7ac, &inj_76, 0x1);
static const uint8_t inj_77 = 0x4d;
taiInjectData(info.modid, 0x00, 0x1f7ea, &inj_77, 0x1);
static const uint8_t inj_78 = 0x4e;
taiInjectData(info.modid, 0x00, 0x1f938, &inj_78, 0x1);
static const uint8_t inj_79 = 0x4f;
taiInjectData(info.modid, 0x00, 0x1fa04, &inj_79, 0x1);
static const uint8_t inj_80 = 0x50;
taiInjectData(info.modid, 0x00, 0x1fa88, &inj_80, 0x1);
static const uint8_t inj_81 = 0x51;
taiInjectData(info.modid, 0x00, 0x22402, &inj_81, 0x1);
*/
  
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {

  if(sceIoOpenHook >= 0) taiHookRelease(sceIoOpenHook, sceIoOpenRef);
  if (io_patch_path) taiInjectRelease(io_patch_path);
  if (io_patch_size) taiInjectRelease(io_patch_size);
  if (peripheral_patch) taiInjectRelease(peripheral_patch);

  return SCE_KERNEL_STOP_SUCCESS;
}
