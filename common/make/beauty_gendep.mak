PYTHON=$(shell which python2)

DEP_DIR = .deps
DEP_SUFFIX = .Po
ifeq ($(MAKECMDGOALS), distclean)
EXTRA_CLEAN += $(DEP_DIR)/*$(DEP_SUFFIX)
endif

ifneq ($(MAKECMDGOALS), distclean)
ifneq ($(MAKECMDGOALS), clean)
DEP_FILES = $(addprefix $(DEP_DIR)/,$(notdir $(patsubst %.o,%$(DEP_SUFFIX),$(C_OBJS))))

ifneq ($(strip $(DEP_FILES)),)
-include $(DEP_FILES)
endif
endif
endif

.phony: distclean
distclean: clean

$(DEP_DIR):
	mkdir $@ | true

quiet_cmd_dep = DEP $(DEP_DIR)/$(notdir $@)
cmd_dep = $(PYTHON) $(PROVITA)/contrib/PC/gendep/gendep.py $(DEP_DIR)/$(notdir $@) $(CC) -MM $(CFLAGS) $<

$(DEP_DIR)/%$(DEP_SUFFIX): %.c $(DEP_DIR)
	@echo $($(quiet)cmd_dep)
	@$(cmd_dep)
