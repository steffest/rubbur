#include "app.h"

BYTE walker_asset;
static UWORD walker_palette[] = {0X000,0X111,0X222,0X434,0X045,0X644,0X069,0Xa54,0Xe09,0X775,0Xb95,0X0bc,0Xf97,0Xcb9,0Xfd9,0Xefd};

void walker_preload(){
    loader_next();
    walker_asset = asset_loadImage("data/walker.planes", 64, 814, 4, MEMF_ANY);
}

void walker(){
    UBYTE loop = 1;
    UWORD frameCount = 0;
    UWORD xScroll = 0;
    UWORD yOffset = 0;
    UBYTE modStep;
    UBYTE step = 0;
    UBYTE i;
    UWORD extentionIndex = 0;
    UWORD copperIndex = 0;
    UWORD gateIndex = 0;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline;
    WORD overlayScrollX = 15;  // Horizontal scroll position (0-15 pixels)
    WORD overlayOffsetWords = 10;  // Coarse horizontal offset in words
    ULONG addr;
    SHORT blend = 255;
    UBYTE isWalking = 1;
    UBYTE delay = 3;

    // palette_set(walker_palette,16); // Moved to after screen setup
    extentionIndex = copper_reset();
    copperIndex = extentionIndex;

    // Set up copper list for scrolling
    // Create a gate to block execution until ready
    gateIndex = copperIndex;
    copperList[copperIndex++] = 0xFFFF; // GATE CLOSED
    copperList[copperIndex++] = 0xFFFE;

    // Set up bitplane pointers
    for (i = 0; i < 4; i++) {
        copperList[copperIndex++] = BPL1PTH + i * 4;
        copperList[copperIndex++] = 0;
        copperList[copperIndex++] = BPL1PTL + i * 4;
        copperList[copperIndex++] = 0;
    }


    // setup background gradient
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0234; // Line 0 RGB=34,51,68

    copperList[copperIndex++] = 0x2801;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0134; // Line 40 RGB=17,51,68

    copperList[copperIndex++] = 0x2901;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0124; // Line 41 RGB=17,34,68

    copperList[copperIndex++] = 0x3101;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0123; // Line 49 RGB=17,34,51

    copperList[copperIndex++] = 0x3501;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0234; // Line 53 RGB=34,51,68

    copperList[copperIndex++] = 0x5401;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0124; // Line 84 RGB=17,34,68

    copperList[copperIndex++] = 0x5801;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0123; // Line 88 RGB=17,34,51

    copperList[copperIndex++] = 0x5F01;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0234; // Line 95 RGB=34,51,68

    copperList[copperIndex++] = 0x6901;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0123; // Line 105 RGB=17,34,51

    copperList[copperIndex++] = 0x8A01;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0122; // Line 138 RGB=17,34,34

    copperList[copperIndex++] = 0x9601;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0111; // Line 150 RGB=17,17,17

    copperList[copperIndex++] = 0x9E01;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0212; // Line 158 RGB=34,17,34

    copperList[copperIndex++] = 0xA101;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0111; // Line 161 RGB=17,17,17

    copperList[copperIndex++] = 0xA401;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0123; // Line 164 RGB=17,34,51

    copperList[copperIndex++] = 0xBD01;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0122; // Line 189 RGB=17,34,34

    copperList[copperIndex++] = 0xD401;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0874; // Line 212 RGB=136,119,68

    copperList[copperIndex++] = 0xD601;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0001; // Line 214 RGB=0,0,17

    copperList[copperIndex++] = 0xD801;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0111; // Line 216 RGB=17,17,17

    copperList[copperIndex++] = 0xDD01;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0112; // Line 221 RGB=17,17,34

    copperList[copperIndex++] = 0xE501;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0434; // Line 229 RGB=68,51,68

    copperList[copperIndex++] = 0xE601;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0212; // Line 230 RGB=34,17,34

    copperList[copperIndex++] = 0xEA01;
    copperList[copperIndex++] = 0xFFFE;
    copperList[copperIndex++] = 0x0180;
    copperList[copperIndex++] = 0x0112; // Line 234 RGB=17,17,34



    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;

    
    screen_reset();
    screen_clear();

    asset_moveToChip(walker_asset);
    screen_disableDoubleBuffer();
    WaitTOF();

    screen_drawAsset(walker_asset,0, 96, 64, 74,0,0);

    // leave black-> fade in
    //palette_set(walker_palette,16);

    // Open the gate
    copperList[gateIndex] = BPLCON1;
    copperList[gateIndex+1] = 0;

    while (loop){
        frameCount++;
        modStep = mod_isStep();

        if (modStep == 15){
            loop = 0;
            modStep = 0;
        }

        if (modStep){

            if (modStep == 5){
                isWalking = 0;

                yOffset = 740;
                screen_drawAsset(walker_asset,0, 96, 64, 74,0,yOffset);
            }

            if (modStep == 7){
                isWalking = 1;
                delay = 1;
            }

            if (isWalking){
                step++;
                if (step > 9){
                    step = 0;
                }
                 yOffset = step*74;
                screen_drawAsset(walker_asset,0, 96, 64, 74,0,yOffset);
            }         
        }

        // fade palette in
        if (blend > 0){
            blend -= 8;
            if (blend < 0) blend = 0;
            palette_fade(walker_palette, 16, blend,0);
        }


        // horizontal scroll
        if (isWalking && frameCount%delay == 0){
            xScroll++;
            overlayScrollX = xScroll & 0x0F;
            overlayOffsetWords = xScroll >> 4;

            copperIndex = extentionIndex;
            copperIndex++;
            copperList[copperIndex++] = overlayScrollX << 4 | overlayScrollX;

            for (i = 0; i < 4; i++) {
                addr = (ULONG)screen_getBitplaneAddress(i);
                addr -= overlayOffsetWords * 2;

                copperIndex++;
                copperList[copperIndex++] = (addr >> 16) & 0xffff;
                copperIndex++;
                copperList[copperIndex++] = addr & 0xffff;
            }   
        }

        WaitTOF();

        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
            asset_free(walker_asset);
            palette_setBlack(32);
        }
    }
}