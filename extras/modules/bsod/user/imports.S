#include "pspimport.s"

IMPORT_START "BlueScreenOfDeathLib", 0x40090000
IMPORT_FUNC "BlueScreenOfDeathLib", 0xCDA22F1B, BlueScreenOfDeathHandler
IMPORT_FUNC "BlueScreenOfDeathLib", 0x245B0ED2, BlueScreenOfDeathRegister