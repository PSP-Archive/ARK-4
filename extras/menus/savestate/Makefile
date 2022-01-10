all:
	make -C kernel
	cp kernel/savestate.prx INGAME.PRX

	make -C user
	cp user/savestate_user.prx MENU.PRX

clean:
	make -C kernel clean
	make -C user clean
	-rm INGAME.PRX
	-rm MENU.PRX
