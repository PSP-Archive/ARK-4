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

NEWIPL = $(ARKROOT)/loader/perma/cipl/new

all:
	$(Q)$(MAKE) -C $(NEWIPL)/ipl_stage2_payload BFLAGS="-DIPL_$(PSP_MODEL) -DPSP_MODEL=$(PSP_MODEL) -DMSIPL=1"
	$(Q)$(MAKE) -C $(NEWIPL)/ipl_stage1_payload BFLAGS="-DIPL_$(PSP_MODEL) -DPSP_MODEL=$(PSP_MODEL) -DMSIPL=1"
	$(Q)dd if=$(NEWIPL)/ipl_stage1_payload/ipl_stage1_payload.bin of=ipl_$(PSP_MODEL).bin
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/psptools/unpack_ipl.py $(ARKROOT)/contrib/PSP/IPL/nandipl_$(PSP_MODEL).bin ipl_$(PSP_MODEL).dec $(XOR_VAL)
	$(Q)dd if=ipl_$(PSP_MODEL).dec of=ipl_$(PSP_MODEL).bin bs=1 seek=49152
	$(Q)$(MAKE) clean -C $(NEWIPL)/ipl_stage2_payload
	$(Q)$(MAKE) clean -C $(NEWIPL)/ipl_stage1_payload
	$(Q)rm ipl_$(PSP_MODEL).dec

clean:
	$(Q)rm payload_*.h ipl_*.bin ipl_*.dec | true
	$(Q)$(MAKE) clean -C payloadex
	$(Q)rm ipl_stage2_payload/ipl_stage2_payload.h | true
