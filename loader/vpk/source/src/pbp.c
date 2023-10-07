#include <vitasdk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sha256.h"
#include "pbp.h"

int HashPbp(char* eboot_file, unsigned char* out_hash) {
  unsigned char wbuf[0x7c0];
  
  // open the pbp file
  SceUID pbp_fd = sceIoOpen(eboot_file, SCE_O_RDONLY, 0);

  // error check
  if(pbp_fd < 0)
    return pbp_fd;
  
  // inital read
  int read_sz = sceIoRead(pbp_fd, wbuf, sizeof(wbuf));
  if(read_sz < sizeof(PbpHeader)) return read_sz;
  
  // calculate data hash size
  size_t hash_sz = (((PbpHeader*)wbuf)->data_psar_ptr + 0x1C0000);

  // initalize hash
  SHA256_CTX ctx;
  sha256_init(&ctx);
  
  // first hash
  sha256_update(&ctx, wbuf, read_sz);
  size_t total_hashed = read_sz;  
  
  do {
    read_sz = sceIoRead(pbp_fd, wbuf, sizeof(wbuf));
    
    if((total_hashed + read_sz) > hash_sz)
      read_sz = (hash_sz - total_hashed); // calculate remaining 
    
    sha256_update(&ctx, wbuf, read_sz);
    total_hashed += read_sz;
    
    if(read_sz < sizeof(wbuf)) // treat EOF as complete
      total_hashed = hash_sz;
    
  } while(total_hashed < hash_sz);
  
  sha256_final(&ctx, out_hash);
  
  sceIoClose(pbp_fd);
  
  return 1;
}
