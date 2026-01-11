OBJS=main.o \
framework/math.o \
framework/system.o \
framework/input.o \
framework/loader.o \
framework/screen.o \
framework/bitplane.o \
framework/display.o \
framework/blitter.o \
framework/copper.o \
framework/sprite.o \
framework/palette.o \
framework/modPlay.o \
framework/assetManager.o \
framework/ptPlayer/ptPlayer.o \
framework/cranker/cranker.o \
framework/cranker/decruncher.o \
framework/effect_screenzoom.o \
framework/support/gcc8_c_support.o \
parts/dude.o \
parts/dude2.o \
parts/dancer.o \
parts/dancer2.o \
parts/walker.o \
parts/oldman.o \
parts/plasma.o \
parts/sequencer.o \
parts/credits.o \
parts/credits2.o \
parts/intro.o \
parts/plasma_asm.o \
parts/outro.o

rsync: $(OBJS)
	slink with rsync.lnk

main.o: main.c
display.o: framework/display.c
modPlay.o: framework/modPlay.c
assetManager.o: framework/assetManager.c
copper.o: framework/copper.c
sprite.o: framework/sprite.c
blitter.o: framework/blitter.c
loader.o: framework/loader.c
screen.o: framework/screen.c
bitplane.o: framework/bitplane.c
system.o: framework/system.c
input.o: framework/input.c
palette.o: framework/palette.c
math.o: framework/math.c
effect_screenzoom.o: framework/effect_screenzoom.c
framework/support/gcc8_c_support.o: framework/support/gcc8_c_support.c
parts/dude.o: parts/dude.c
parts/dude2.o: parts/dude2.c
parts/dancer.o: parts/dancer.c
parts/dancer2.o: parts/dancer2.c
parts/walker.o: parts/walker.c
parts/oldman.o: parts/oldman.c
parts/plasma.o: parts/plasma.c
parts/sequencer.o: parts/sequencer.c
parts/credits.o: parts/credits.c
parts/credits2.o: parts/credits2.c
parts/intro.o: parts/intro.c
parts/outro.o: parts/outro.c

parts/plasma_asm.o: parts/plasma_asm.s
	PhxAss parts/plasma_asm.s to parts/plasma_asm.o

framework/cranker/cranker.o: framework/cranker/cranker.c
framework/cranker/decruncher.o: framework/cranker/decruncher.asm
	PhxAss framework/cranker/decruncher.asm to framework/cranker/decruncher.o

   