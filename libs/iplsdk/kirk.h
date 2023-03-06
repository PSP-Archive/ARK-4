#ifndef __KIRK_H__
#define __KIRK_H__

void KirkReset();
int KirkCmd1(void *dest, void *src);
void KirkCmdF();
int kirkDecryptAes(u8 *out, u8 *data, u32 size, u8 key_idx);

#endif
