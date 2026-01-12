RUBBUR

An OldSkool Demo for Amiga OCS  
Runs on Amiga 500 with 512k FastRAM and beyond.

[![RUBBUR](https://img.youtube.com/vi/cc8f7zg3-v8/0.jpg)](https://www.youtube.com/watch?v=cc8f7zg3-v8)

Video on https://youtu.be/cc8f7zg3-v8

Written in hardware banging C, with a little pinch of assembler.  
Compile with SAS-C on Amiga. (just run smake)  
It uses PhxAss to compile the assembler parts.  

Or cross-compile with [vamos](https://lallafa.de/blog/amiga-projects/amitools/vamos/) on osX or windows.  

Released at RSYNC Demoparty 2026

# Running
You find the executable, ADF disk and MOD file in the "Release" folder.

# Building
I probably should include my full setup on the Amiga side, but for now: go find your (emulated) Amiga and install

- [SAS/C compiler](https://www.amigaclub.be/_files/SASC-6.0-6.5.zip)
- [PhxAss assembler](https://aminet.net/package/dev/asm/PhxAss)

-> navigate to the "rubbur" folder and run "smake"
It builds the executable to the file "a"

Compilation is done on amiga, but you can also cross-compile with [vamos](https://lallafa.de/blog/amiga-projects/amitools/vamos/) on osX or windows.
for that, you'll need to update the paths to your amiga files (where you put Sas/C and PhxAss) in the ".vamosrc" file.

The mod is included in the "release" folder. I use [SplitMod](https://aminet.net/package/util/cli/SplitMOD) to splt it into sample data and song data, so the song data can be loaded into Fast RAM.

# Things I learned

- how to flip an image horizontally using code
- how to use functions in assembler to speed up operations and call them from C
- how to scroll a screen horizontally
- how to use vamos to run the SAS/C compiler on osX and windows
- I moved from shrinkler to cranker for compression. Shrinkler compresses better, but cranker is faster to decompress.
- in the end, I avoided compression alltogether. If all assets fit on a single floppy, it's simpeler and visualy nicer to keep them uncompressed.
- another thing I figured out but did't use, was how to use the assembler INCBIN thing to include binary data in the executable. Good to know if I ever want a single file demo. For now, the approach to have a light startup exe to display a loading screen and post-load the data is fine.

# Things I want/need to improve in future productions
- get a hold of timing ... Sync is all over the place and behaves differently on different machines. No doubt my code is still too slow, but at the VERY least I should measure how many cycles each part of the code takes. This is the reason why most effects run a bit faster on a 1200. I beat-synced the thing to be in sync on my machine, but the timing will be slighty of on other machines. This sucks ... To be improved in the next production.






