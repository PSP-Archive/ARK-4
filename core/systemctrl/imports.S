	.set noreorder

#include "pspstub.s"

	STUB_START "SysclibForKernel",0x00090011,0x00210005
	STUB_FUNC  0x7DEE14DE,__udivdi3
	STUB_FUNC  0xDF17F4A2,__udivmoddi4
	STUB_FUNC  0x5E8E5F42,__umoddi3
	STUB_FUNC  0xC2145E80,snprintf
	STUB_END
