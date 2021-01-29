#include <sdk.h>
#include "kxploit.h"
#include "macros.h"

void initKxploit(KxploitFunctions*)__attribute__((section(".text.startup")));
void initKxploit(KxploitFunctions* kf){
    kf->stubScanner = &stubScanner;
    kf->doExploit = &doExploit;
    kf->executeKernel = &executeKernel;
    kf->repairInstruction = &repairInstruction;
}
