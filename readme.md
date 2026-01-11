RUBBUR

An OldSkool Demo for Amiga OCS
Runs on Amiga 500 with 515k FastRAM and beyond.

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
I probably should include my full setup on the Amiga side, but for now:
- SAS/C compiler
- PhxAss assembler
- 
-> run "smake"
It builds the executable to the file "a"

Compilation is done on amiga, but you can also cross-compile with vamos on osX or windows.
for that, you'll need to update the paths to your amiga files in the ".vamosrc" file.


# Things I learned

- how to flip an image horizontally using the blitter
- how to use functions in assembler to speed up operations and call them from C
- how to scroll a screen horizontally
- how to use vamos to run the SAS/C compiler on osX and windows
- I moved from shrinkler to cranker for compression. Shrinkler compresses better, but cranker is faster to decompress.

# Things I want/need to improve in future productions
- get a hold of timing ... Sync is all over the place and behaves differently on different machines. No doubt my code is still too slow, but at the VERY least I should measure how many cycles each part of the code takes.






