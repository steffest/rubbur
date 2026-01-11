#include "app.h"
#include "clib/graphics_protos.h"
#include "framework/assetManager.h"
#include <hardware/custom.h>

extern struct Custom far custom;

BYTE dancer2_asset;
BYTE notes_asset;
extern BYTE *bar_overlay;

static UWORD back_gradient[] = {
0x228,
0X338,
0X449,
0X54A,
0X85B,
0X95B,
0XB5B,
0XB5B,
0X205,
0X348,
0X033,
0X237
};

/*
#228,
#338,
#449,
#54A,
#85B,
#95B,
#B5B,
#B5B,
#205,
#348,
#033,
#237<
*/

static UWORD dancer2_palette[] = {
    0x117,0x220,
    0x505,0x220, // Copied from 0,1 to make BP2 invisible, excpent for the shadow
    
    0xe13,0xa76,
    0xe13,0xa76, // Copied from 4,5
    
    // 8-15: Lighter Tints of 0-7 (Blended 80% with White)
    0x117,0xCCC,
    0xDCD,0xCCC,
    0xFCD,0xEDD,
    0xFCD,0xEDD,

    0xe25,0x37a,
    0xe25,0x37a, // Copied from 16,17

    0xe94,0xfd9,
    0xe94,0xfd9, // Copied from 20,21

    // 24-31: Lighter Tints of 16-23 (Blended 80% with White)
    0xFCD,0xDDE,
    0xFCD,0xDDE,
    0xFED,0xFFE,
    0xFED,0xFFE
};

void dancer2_preload(){
    loader_next();
    dancer2_asset = asset_loadImage("data/dancer2.planes", 144, 555, 3, MEMF_ANY);
    loader_next();
    notes_asset = asset_loadImage("data/notes.planes", 320, 128, 1, MEMF_ANY);
}

void dancer2(){
    UBYTE loop = 1;
    UWORD frameCount = 0;
    UWORD yOffset = 0;
    UBYTE modStep;
    UBYTE step = 0;
    UBYTE imageFrame = 0;
    UWORD extentionIndex = 0;
    UWORD copperIndex = 0;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline;
    UWORD wobbleIndex = 0;

    struct DScreen *screen;
    ULONG plane2Base;
    ULONG addr;
    UWORD src_y;
    UWORD dist;
    UWORD val;
    UWORD val2;
    UBYTE scrollY = 0;
    int gradIdx;
    UWORD bgCol, lightCol, darkCol;
    UBYTE blend;
    UBYTE dancerWobble = 0;

    palette_set(dancer2_palette,32);
    extentionIndex = copper_reset();
    copperIndex = extentionIndex;
    screen_reset();
    screen_clear();

    generate_bar_overlay();
    asset_moveToChip(notes_asset);
    // copy notes to overlay
    if (bar_overlay) {
        // Blit 1: Lines 0-127
        WaitBlit();
        custom.bltcon0 = 0x09F0; // A->D Copy
        custom.bltcon1 = 0;
        custom.bltapt = (APTR)getAssetData(notes_asset);
        custom.bltdpt = bar_overlay;
        custom.bltamod = 0;
        custom.bltdmod = 0;
        custom.bltsize = (128 << 6) | 20; 

        // Blit 2: Lines 128-255
        WaitBlit();
        custom.bltcon0 = 0x09F0;
        custom.bltcon1 = 0;  
        custom.bltapt = (APTR)getAssetData(notes_asset);
        custom.bltdpt = bar_overlay + (128 * 40);
        custom.bltamod = 0;
        custom.bltdmod = 0;
        custom.bltsize = (128 << 6) | 20;

        // Blit 3: Lines 256-383
        WaitBlit();
        custom.bltcon0 = 0x09F0;
        custom.bltcon1 = 0;  
        custom.bltapt = (APTR)getAssetData(notes_asset);
        custom.bltdpt = bar_overlay + (256 * 40);
        custom.bltamod = 0;
        custom.bltdmod = 0;
        custom.bltsize = (128 << 6) | 20;

        // Blit 4: Lines 384-511
        WaitBlit();
        custom.bltcon0 = 0x09F0;
        custom.bltcon1 = 0;  
        custom.bltapt = (APTR)getAssetData(notes_asset);
        custom.bltdpt = bar_overlay + (384 * 40);
        custom.bltamod = 0;
        custom.bltdmod = 0;
        custom.bltsize = (128 << 6) | 20;
        WaitBlit();
    }

    asset_free(notes_asset);
    asset_moveToChip(dancer2_asset);

    // Use override to display the bar_overlay on Bitplane 3 
    if (bar_overlay) {
        screen_overrideBitPlane(3, bar_overlay);
    }

    screen_disableDoubleBuffer();
    WaitTOF();

    screen_drawAssetPlane(dancer2_asset, 0, 80, 20, 144, 183, 0, 0, 0);
    screen_drawAssetPlane(dancer2_asset, 1, 88, 15, 144, 183, 0, 0, 0);
    screen_drawAssetPlane(dancer2_asset, 2, 80, 20, 144, 183, 1, 0, 0);
    screen_drawAssetPlane(dancer2_asset, 4, 80, 20, 144, 183, 2, 0, 0);

    screen = screen_getScreen();
    plane2Base = (ULONG)screen_getBitplaneAddress(1);

    for (scanline = 50; scanline < 255; scanline++) {

        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
        copperList[copperIndex++] = 0xfffe;

        gradIdx = ((scanline - 50) * 12) / 206;
        if (gradIdx > 11) gradIdx = 11;
        bgCol = back_gradient[gradIdx];
        copperList[copperIndex++] = 0x0180; // COLOR00
        copperList[copperIndex++] = bgCol;

        // Darker version for COLOR02 and 10 (shadow)
        blend = 64;
        if (scanline > 200) blend = 128;
        if (scanline > 240) blend = 192;
        darkCol = palette_blendColor(bgCol, 0x000, blend); 
        copperList[copperIndex++] = 0x0184; // COLOR02
        copperList[copperIndex++] = darkCol;
        copperList[copperIndex++] = 0x0194; // COLOR10
        copperList[copperIndex++] = darkCol;

        blend = 64;
        if (scanline > 200) blend = 40;
        if (scanline > 240) blend = 32;
        // Lighter version for COLOR08 (dots)
        lightCol = palette_blendColor(bgCol, 0xFFF, blend); // Blend with white
        copperList[copperIndex++] = 0x0190; // COLOR08
        copperList[copperIndex++] = lightCol;

        dist = (scanline > 249) ? (scanline - 249) : (249 - scanline);
        if (dist > 63) dist = 63;
        val = math_sin16(scanline);
        val = (val * dist) >> 6;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = val << 4 ;

        // Squash logic for Bitplane 2 
        // Target Range: 113 to 205 (Height 92) -> VPOS 157 to 249
        // Squash source [80..205] into [113..205]
        if (scanline <= 249) {
             if (scanline < 147) {
                 src_y = 1; // Clamp top to safe area
             } else {
                 // Increased squash factor: 200/100 = 2.0
                 // src_y = 205 - ((249 - scanline) * 200) / 100;
                 // Maps [147..249] (102 lines) to source [1..205]
                 src_y = 205 - ((LONG)(249 - scanline) * 200) / 100;
             }
             
             addr = plane2Base + src_y * 40; // 40 bytes per line
             
             copperList[copperIndex++] = BPL2PTH;
             copperList[copperIndex++] = (addr >> 16);
             copperList[copperIndex++] = BPL2PTL;
             copperList[copperIndex++] = (addr & 0xFFFF);
        }
    }

    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;

    while (loop){
        frameCount++;
        modStep = mod_isStep();

        if (modStep == 15){
            loop = 0;
            modStep = 0;
        }

        if (modStep > 8){
            modStep = 0;
            // this is a beat-sync for re-used mod patterns later in the demo
        }

        if (modStep){
            step++;
            if (step > 1){
               imageFrame++;
                if (imageFrame > 3){
                    imageFrame = 0;
                }
                yOffset = imageFrame*185;
                if (yOffset > 370) yOffset = 185;

                screen_drawAssetPlane(dancer2_asset, 0, 80, 20, 144, 183, 0, 0, yOffset);
                screen_drawAssetPlane(dancer2_asset, 1, 88, 15, 144, 183, 0, 0, yOffset);
                screen_drawAssetPlane(dancer2_asset, 2, 80, 20, 144, 183, 1, 0, yOffset);
                screen_drawAssetPlane(dancer2_asset, 4, 80, 20, 144, 183, 2, 0, yOffset);
            }

            
            if (step == 7) dancerWobble = 1;
            if (step == 9) dancerWobble = 0;
            if (step == 15) dancerWobble = 1;
            if (step == 17) dancerWobble = 0;
            if (step == 23) dancerWobble = 1;
            if (step == 25) dancerWobble = 0;
            if (step == 29) dancerWobble = 1;
        }

        scrollY++;
        if (bar_overlay) {
             screen_overrideBitPlane(3, bar_overlay + (scrollY * 40));
        }

        copperIndex = extentionIndex;
        // Re-calculate plane2Base here if double buffering is active, though we disabled it.
        // plane2Base = (ULONG)screen_getBitplaneAddress(1); 
        
        for (scanline = 50; scanline < 255; scanline++) {
            copperIndex += 11; // Skip Wait(2) + Colors + BPLCON1(1)

            dist = (scanline > 249) ? (scanline - 249) : (249 - scanline);
            if (dist > 63) dist = 63;
            val = math_sin16(scanline + wobbleIndex);
            val = (val * dist) >> 6;
            
            if (dancerWobble){
                val2 = math_sin16((scanline + wobbleIndex) * 2);
                val2 = (val2 * dist) >> 6;
            }else{
                val2 = 0;
            }

            copperList[copperIndex++] = val << 4 | val2;
            
            if (scanline <= 249) {
                copperIndex += 4; // Skip BPL2PTH/L instructions
            }
        }

        wobbleIndex += 4;

        WaitTOF();

        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
            palette_setBlack(32);
            screen_reset();
            screen_clear();
            asset_free(dancer2_asset);
        }
    }
}