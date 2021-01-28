all:
	make -C kernel
	psp-packer kernel/savestate.prx
	cp kernel/savestate.prx SAVE.PRX

	make -C user
	psp-packer user/savestate_user.prx
	cp user/savestate_user.prx SUSER.PRX

clean:
	make -C kernel clean
	make -C user clean
	-rm SAVE.PRX
	-rm SUSER.PRX
