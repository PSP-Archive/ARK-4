#include "model.h"

#include <syscon.h>
#include <sysreg.h>

const PspModelIdentity g_psp_models[] = {
    { "TA-079v1",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00140000, 0x00010600, 0x00000103, "200409230625", "First" },
    { "TA-079v2",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00140000, 0x00020600, 0x00000103, "200410071128", "First" },
    { "TA-079v3",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00140000, 0x00030600, 0x00000103, "200411290757", "First" },
    { "TA-079v4",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00200000, 0x00030600, 0x00000103, "200411290757", "First" },
    { "TA-079v5",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00200000, 0x00040600, 0x00000103, "200504040852", "First" },
    { "TA-081v1",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00300000, 0x00040600, 0x00000103, "200504040852", "First" },
    { "TA-081v2",   "FAT 1000",     "01g", PSP_MODEL_01G, 0x00300000, 0x00040600, 0x00000104, "200504040852", "First" },
    { "TA-082",     "FAT 1000",     "01g", PSP_MODEL_01G, 0x00400000, 0x00114000, 0x00000112, "200509260441", "Legolas1" },
    { "TA-086",     "FAT 1000",     "01g", PSP_MODEL_01G, 0x00400000, 0x00121000, 0x00000112, "200512200558", "Legolas2" },
    { "TA-085v1",   "SLIM 2000",    "02g", PSP_MODEL_02G, 0x00500000, 0x0022B200, 0x00000123, "200704161420", "Frodo" },
    { "TA-085v2",   "SLIM 2000",    "02g", PSP_MODEL_02G, 0x00500000, 0x00234000, 0x00000123, "200710022249", "Frodo" },
    { "TA-088v1",   "SLIM 2000",    "02g", PSP_MODEL_02G, 0x00500000, 0x00243000, 0x00000123, "200711022212", "Frodo" },
    { "TA-088v2",   "SLIM 2000",    "02g", PSP_MODEL_02G, 0x00500000, 0x00243000, 0x00000123, "200711022212", "Frodo" },
    { "TA-088v3",   "SLIM 2000",    "02g", PSP_MODEL_02G, 0x00600000, 0x00243000, 0x00000123, "200711022212", "Frodo" },
    { "TA-090v1",   "SLIM 2000",    "02g", PSP_MODEL_02G, 0x00500000, 0x00243000, 0x00000132, "200711022212", "Frodo" },
    { "TA-090v2",   "BRIGHT 3000",  "03g", PSP_MODEL_03G, 0x00600000, 0x00263100, 0x00000132, "200803241434", "Samwise" },
    { "TA-090v3",   "BRIGHT 3000",  "03g", PSP_MODEL_03G, 0x00600000, 0x00263100, 0x00000133, "200803241434", "Samwise" },
    { "TA-092",     "BRIGHT 3000",  "03g", PSP_MODEL_03G, 0x00600000, 0x00285000, 0x00000133, "200902091613", "Samwise" },
    { "TA-093v1",   "BRIGHT 3000",  "04g", PSP_MODEL_04G, 0x00810000, 0x002C4000, 0x00000141, "200904011911", "Samwise VA2" },
    { "TA-093v2",   "BRIGHT 3000",  "04g", PSP_MODEL_04G, 0x00810000, 0x002C4000, 0x00000143, "200904011911", "Samwise VA2" },
    { "TA-091",     "GO N1000",     "05g", PSP_MODEL_05G, 0x00720000, 0x00304000, 0x00000133, "200904092125", "Strider" },
    { "TA-095v0?",  "BRIGHT 3000",  "09g", PSP_MODEL_09G, 0x00800000, 0x002E4000, 0x00000154, "201006081334", "Samwise VA2" },
    { "TA-095v1",   "BRIGHT 3000",  "09g", PSP_MODEL_09G, 0x00810000, 0x002E4000, 0x00000154, "201006081334", "Samwise VA2" },
    { "TA-095v2",   "BRIGHT 3000",  "09g", PSP_MODEL_09G, 0x00820000, 0x002E4000, 0x00000154, "201006081334", "Samwise VA2" },
    { "TA-095v3",   "BRIGHT 3000",  "07g", PSP_MODEL_07G, 0x00810000, 0x012E4000, 0x00000154, "201006081334", "Samwise VA2" },
    { "TA-095v4",   "BRIGHT 3000",  "07g", PSP_MODEL_07G, 0x00820000, 0x012E4000, 0x00000154, "201006081334", "Samwise VA2" },
    { "TA-096",     "STREET E1000", "11g", PSP_MODEL_11G, 0x00900000, 0x00403000, 0x00000154, "201105092045", "Bilbo" },
    { "TA-097",     "STREET E1000", "11g", PSP_MODEL_11G, 0x00900000, 0x00403000, 0x00000154, "201105092045", "Bilbo" },
};

#define NUM_PSP_MODELS  (sizeof(g_psp_models)/sizeof(*g_psp_models))

const PspModelIdentity *model_get_identity(void)
{
    static const PspModelIdentity *s_identity = NULL;

    if (s_identity) {
        return s_identity;
    }

    uint32_t baryon = syscon_get_baryon_version();
    uint32_t tachyon = sysreg_get_tachyon_version();
    unsigned int pommel = 0;
    syscon_get_pommel_version(&pommel);

    for (size_t i = 0; i < NUM_PSP_MODELS; ++i) {
        if (g_psp_models[i].baryon == baryon && g_psp_models[i].tachyon == tachyon && g_psp_models[i].pommel == pommel) {
            s_identity = &g_psp_models[i];
            return s_identity;
        }
    }

    return NULL;
}
