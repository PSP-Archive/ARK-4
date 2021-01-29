quiet_cmd_cc_o_c = CC $<
cmd_cc_o_c = $(CC) $(CFLAGS) -c $< -o $@

CFLAGS := $(addprefix -I,$(INCDIR)) $(CFLAGS)

%.o: %.c
	@echo $($(quiet)cmd_cc_o_c)
	@$(cmd_cc_o_c)

quiet_cmd_ar = AR $@
cmd_ar = psp-ar q $(TARGET_LIB) $(OBJS)

quiet_cmd_ranlib = RANLIB $@
cmd_ranlib = psp-ranlib $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	@echo $($(quiet)cmd_ar)
	@$(cmd_ar)
	@echo $($(quiet)cmd_ranlib)
	@$(cmd_ranlib)

.phony: clean

clean:
	$(Q)rm -f $(OBJS) $(TARGET_LIB) $(EXTRA_CLEAN)
