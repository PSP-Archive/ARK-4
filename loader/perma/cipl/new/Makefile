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
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/ipltools/unpack_ipl.py $(ARKROOT)/contrib/PSP/IPL/nandipl_$(PSP_MODEL).bin ipl_$(PSP_MODEL).dec $(XOR_VAL)
	$(Q)dd if=ipl_stage1_payload/ipl_stage1_payload.bin of=ipl_$(PSP_MODEL).bin
	$(Q)dd if=ipl_$(PSP_MODEL).dec of=ipl_$(PSP_MODEL).bin bs=1 seek=49152
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/ipltools/make_ipl.py ipl_$(PSP_MODEL).bin cipl_$(PSP_MODEL).bin reset_block 0x40e0000
	$(Q)bin2c cipl_$(PSP_MODEL).bin cipl_$(PSP_MODEL).h cipl_$(PSP_MODEL)
	$(Q)bin2c $(ARKROOT)/contrib/PSP/IPL/nandipl_$(PSP_MODEL).bin origipl_$(PSP_MODEL).h origipl_$(PSP_MODEL)
	$(Q)$(MAKE) clean -C ipl_stage2_payload
	$(Q)$(MAKE) clean -C ipl_stage1_payload

clean:
	$(Q)rm origipl_*.h cipl_*.h cipl_*.bin ipl_*.bin ipl_*.dec | true
	$(Q)rm ipl_stage2_payload/ipl_stage2_payload.h | true
