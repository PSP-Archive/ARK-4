all:
	$(Q)$(MAKE) -C ipl_stage2_payload BFLAGS="-DIPL_$(PSP_MODEL) -DPSP_MODEL=$(PSP_MODEL)"
	$(Q)$(MAKE) -C ipl_stage1_payload BFLAGS="-DIPL_$(PSP_MODEL) -DPSP_MODEL=$(PSP_MODEL)"
	$(Q)mv ipl_stage1_payload/ipl_stage1_payload.h payload_$(PSP_MODEL).h
	$(Q)rm ipl_stage2_payload/ipl_stage2_payload.h
	$(Q)$(MAKE) clean -C ipl_stage2_payload
	$(Q)$(MAKE) clean -C ipl_stage1_payload

clean:
	$(Q)rm payload_*.h
	$(Q)$(MAKE) clean -C payloadex