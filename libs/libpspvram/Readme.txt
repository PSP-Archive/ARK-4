libpspvram
Copyright (c) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
http://wordpress.fx-world.org


This library provides two different versions of a dynamic VRAM allocation manager, namely libpspvram and libpspvalloc.

The two versions differ in the following specifics:
-valloc uses dynamic system ram allocation for the management structures (hence may contribute to fragmentation and is slower for allocations/frees)
-valloc aligns on 16byte default, while vram allocates on 512byte default (hence vram has a little overhead for allocations not a multiple of 512byte)
-vram is much faster than valloc
-vram doesn't provide "vgetMemorySize" any more, as this function has nothing really to do with the MMU itself

libpspvram is the recommended version to use, unless you find real reasons to stay with libpspvalloc.

To install type:
make && make install

Then just link -lpspvram or -lpspvalloc in your makefile and include <vram.h> or <valloc.h> in your source code.
