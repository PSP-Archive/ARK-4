PYTHON=$(shell which python3)

PSPBTCNF_TARGETS = psvbtinf psvbtcnf
PSPBTCNF_OBJS = $(addsuffix .bin,$(PSPBTCNF_TARGETS))

.PHONY: clean

all: $(PSPBTCNF_OBJS)

clean:
	$(Q)rm -f $(PSPBTCNF_OBJS)

quiet_cmd_btcnf = BTCNF $<
cmd_btcnf = $(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build $<

%.bin:%.txt
	@echo $($(quiet)cmd_btcnf)
	@$(cmd_btcnf)

#include ../../../common/make/quiet.mak
include $(ARKROOT)/common/make/quiet.mak
