#ifndef AT3_H
#define AT3_H

#ifdef __cplusplus
extern "C" {
#endif

void resetAt3Data();
void setAt3Data(char* data, int size, int* abortVar, int delay);
int AT3_T(SceSize argc, void* argv);

#ifdef __cplusplus
}
#endif

#endif
