#include <vitasdk.h>

static int loadScePaf() {
	static uint32_t argp[] = { 0x180000, -1, -1, 1, -1, -1 };

	int result = -1;

	uint32_t buf[4];
	buf[0] = sizeof(buf);
	buf[1] = (uint32_t)&result;
	buf[2] = -1;
	buf[3] = -1;

	return sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(argp), argp, buf);
}

static int unloadScePaf() {
	uint32_t buf = 0;
	return sceSysmoduleUnloadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, 0, NULL, &buf);
}

int promoteCma(const char *path, const char *titleid, int type) {
  int res;
  
  ScePromoterUtilityImportParams promoteArgs;
  memset(&promoteArgs,0x00,sizeof(ScePromoterUtilityImportParams));
  strncpy(promoteArgs.path,path,0x7F);
  strncpy(promoteArgs.titleid,titleid,0xB);
  promoteArgs.type = type;
  promoteArgs.attribute = 0x1;

  res = loadScePaf();
	if (res < 0) {
		return res;
	}

  res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	if (res < 0) {
		return res;
	}

  res = scePromoterUtilityInit();
	if (res < 0) {
		return res;
	}

  res = scePromoterUtilityPromoteImport(&promoteArgs);
	if (res < 0) {
		return res;
	}

  res = scePromoterUtilityExit();
	if (res < 0) {
		return res;
	}

  res = sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	if (res < 0) {
		return res;
	}


  res = unloadScePaf();
  if (res < 0)
	if (res < 0) {
		return res;
	}

  return res;
}