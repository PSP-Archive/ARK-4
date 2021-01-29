quiet_cmd_compile = CC $<
cmd_compile = $(CC) $(CFLAGS) -c $< -o $@
quiet_cmd_link = LINK $@
cmd_link = $(LD) -T $(LINKFILE) $^ -o $@ $(LIBS)
quiet_cmd_genlink = GENLINK $@ $(LOADADDR)

%.o: %.c
	@echo $($(quiet)cmd_compile)
	@$(cmd_compile)

%.elf: %.o
	@echo $($(quiet)cmd_link)
	@$(cmd_link)
