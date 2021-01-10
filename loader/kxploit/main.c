#include <sdk.h>
#include "kxploit.h"
#include "flashpatch.h"
#include "macros.h"

void initKxploit()__attribute__((section(".text.startup")));
void initKxploit(){
	kxf->stubScanner = &stubScanner;
	kxf->doExploit = &doExploit;
	kxf->executeKernel = &executeKernel;
	kxf->repairInstruction = &repairInstruction;
}
