#ifndef AT3_H
#define AT3_H

#ifdef __cplusplus
extern "C" {
#endif

void pspavSetAt3Data(char* data, int size, int* abortVar, int delay);
void pspavResetAt3Data();
int pspavPlayAT3(SceSize argc, void* argv);

#ifdef __cplusplus
}
#endif

#endif
