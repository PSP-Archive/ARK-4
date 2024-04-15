#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

enum PspModel
{
    PSP_MODEL_01G,
    PSP_MODEL_02G,
    PSP_MODEL_03G,
    PSP_MODEL_04G,
    PSP_MODEL_05G,
    PSP_MODEL_07G,
    PSP_MODEL_09G,
    PSP_MODEL_11G
};

typedef struct {
    const char *motherboard;
    const char *model_str;
    const char *gen_str;
    enum PspModel model;
    uint32_t tachyon;
    uint32_t baryon;
    uint32_t pommel;
    const char *baryon_timestamp;
    const char *codename;
} PspModelIdentity;

const PspModelIdentity *model_get_identity(void);

#ifdef __cplusplus
}
#endif //__cplusplus
