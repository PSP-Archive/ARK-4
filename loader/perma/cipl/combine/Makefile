MODULE_PATH = ..

PYTHON=$(shell which python3)

all:
	$(PYTHON) ./combine.py ipl_header_01g.bin ../mainbinex/patch.bin ipl_block_01g.bin
	$(PYTHON) ./combine.py -l 0x24000 ipl_header_02g.bin ../mainbinex/patch.bin ipl_block_02g.bin
	bin2c ipl_block_01g.bin ipl_block_01g.h ipl_block_01g
	bin2c ipl_block_02g.bin ipl_block_large.h ipl_block_large
	sed "s/((aligned(16)))/((aligned(64)))/" ipl_block_large.h > $(MODULE_PATH)/ipl_block_large.h
	sed "/int/d" ipl_block_01g.h > $(MODULE_PATH)/ipl_block_01g.h

clean:
	@rm -f $(MODULE_PATH)/ipl_block_01g.h $(MODULE_PATH)/ipl_block_large.h 
	@rm -f ipl_block_01g.bin ipl_block_02g.bin kbooti_ipl_block_01g.bin ipl_block_01g.h ipl_block_large.h 
