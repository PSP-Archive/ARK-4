ifeq ($(V), 1)
	quiet = 
	Q =
else
	quiet = quiet_
	Q = @
endif

quiet_cmd_cc_o_c = CC $<
cmd_cc_o_c = $(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	@echo $($(quiet)cmd_cc_o_c)
	@$(cmd_cc_o_c)

quiet_cmd_cc_o_S = CC $<
cmd_cc_o_S = $(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo $($(quiet)cmd_cc_o_c)
	@$(cmd_cc_o_c)

quiet_cmd_build_exports = BUILD-EXPORTS $@
cmd_build_exports = psp-build-exports -b $< > $@

%.c: %.exp
	@echo $($(quiet)cmd_build_exports)
	@$(cmd_build_exports)

quiet_cmd_link = LINKING $@
cmd_link = $(LINK.c) $^ $(LIBS) -o $@

quiet_cmd_fixup = FIXUP $@
cmd_fixup = $(FIXUP) $@

ifeq ($(NO_FIXUP_IMPORTS), 1)
$(TARGET).elf: $(OBJS) $(EXPORT_OBJ)
	@echo $($(quiet)cmd_link)
	@$(cmd_link)
else
$(TARGET).elf: $(OBJS) $(EXPORT_OBJ)
	@echo $($(quiet)cmd_link)
	@$(cmd_link)
	@echo $($(quiet)cmd_fixup)
	@$(cmd_fixup)
endif
