ifeq ($(PSP_MODEL), 01G)
XOR_VAL = 
else
ifeq ($(PSP_MODEL), 02G)
XOR_VAL = 
else
ifeq ($(PSP_MODEL), 05G)
XOR_VAL = --xor 2
else
XOR_VAL = --xor 1
endif
endif
endif

all:
	$(Q)$(MAKE) -C ipl_stage2_payload BFLAGS="-DIPL_$(PSP_MODEL) -DPSP_MODEL=$(PSP_MODEL)"
	$(Q)$(MAKE) -C ipl_stage1_payload BFLAGS="-DIPL_$(PSP_MODEL) -DPSP_MODEL=$(PSP_MODEL)"
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/psptools/unpack_ipl.py $(ARKROOT)/contrib/PSP/nandipl_$(PSP_MODEL).bin ipl_$(PSP_MODEL).dec $(XOR_VAL)
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/psptools/pack_ipl.py ipl_stage1_payload/ipl_stage1_payload.bin@0x40e0000 ipl_$(PSP_MODEL).dec@0x40ec000 nandcipl_$(PSP_MODEL).bin 0x40e0000
	$(Q)bin2c nandcipl_$(PSP_MODEL).bin payload_$(PSP_MODEL).h payload_$(PSP_MODEL)
	$(Q)$(MAKE) clean -C ipl_stage2_payload
	$(Q)$(MAKE) clean -C ipl_stage1_payload

clean:
	$(Q)rm payload_*.h nandcipl_*.bin ipl_*.dec | true
	$(Q)$(MAKE) clean -C payloadex
	$(Q)rm ipl_stage2_payload/ipl_stage2_payload.h | true