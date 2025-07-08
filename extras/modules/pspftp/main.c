#include <pspsdk.h>

PSP_MODULE_INFO("FTPlibPSP", PSP_MODULE_SINGLE_LOAD|PSP_MODULE_SINGLE_START, 1, 0);

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(){
    return 0;
}
