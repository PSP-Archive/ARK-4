quiet_cmd_clean = CLEAN
cmd_clean = rm -f $(OBJS) $(TARGET) *~

clean:
	@echo $($(quiet)cmd_clean)
	@$(cmd_clean)

quiet_cmd_ar = AR $@
cmd_ar = psp-ar q $(TARGET) $(OBJS)

quiet_cmd_ranlib = RANLIB $@
cmd_ranlib = psp-ranlib $(TARGET)

$(TARGET): $(OBJS)
	@echo $($(quiet)cmd_ar)
	@$(cmd_ar)
	@echo $($(quiet)cmd_ranlib)
	@$(cmd_ranlib)

quiet_cmd_cc_o_c = STUB $@
cmd_cc_o_c = $(CC) $(CFLAGS) -DF_$* $< -c -o $@

$(OBJS): $(STUBSRC)
	@echo $($(quiet)cmd_cc_o_c)
	@$(cmd_cc_o_c)
