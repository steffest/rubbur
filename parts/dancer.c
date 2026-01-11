#include "app.h"
#include "framework/assetManager.h"
#include <hardware/custom.h>

extern struct Custom far custom;

// TODO:
// Music notes ovetlay
// reuse bar_overlay;
// scroll notes


BYTE dancer_asset;
BYTE dancer_mirror_asset;
BYTE dots_asset = -1;
extern BYTE *bar_overlay;

static UWORD dancer_palette[] = {
    // 0-7: Base
    0x228,0x111,
    0x909,0x111, // Shadow at 2, 3 is copy of 1
    0x533,0x965,
    0x533,0x965, // 6,7 copies of 4,5
    
    // 8-15: Lighter Tints of 0-7
    0x228,0x444,
    0xe1e,0x444, 
    0xa45,0xe77,
    0xa45,0xe77,

    // 16-23: High colors (Originals)
    0xe25,0x37a,
    0xe2f,0x37f,
    0xe94,0xfd9,
    0xe9f,0xfd9,

    // 24-31: Lighter Tints of 16-23 
    0xf36,0x59e,
    0xf3d,0x59f,
    0xfb5,0xffa,
    0xfb9,0xffa
};

static UWORD back_gradient[] = {
0x228,
0x338,
0x449,
0x54A,
0x85B,
0x95B,
0xB5B,
0xB5B,
0xB34,
0xFA8,
0xF98,
0xe76
};




void dancer_preload(){
    loader_next();
    dancer_asset = asset_loadImage("data/dancer.planes", 128, 558, 3, MEMF_ANY);
    loader_next();
    dots_asset = asset_loadImage("data/dots.planes", 320, 256, 1, MEMF_ANY);
}

void dancer(){
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
    UBYTE active_asset;

    // palette_set(dancer_palette,32);
    extentionIndex = copper_reset();
    copperIndex = extentionIndex;
    screen_reset();
    screen_clear();

    dancer_mirror_asset = asset_flipImageHorizontal(dancer_asset);
    // we're low on chip memory here.
    // there's a change this fails, but then the index will be the same as dancer_asset
    // so we can use it as a fallback

    generate_bar_overlay();
    asset_moveToChip(dots_asset);
    // copy dots to overlay
    if (bar_overlay) {
        WaitBlit();
        custom.bltcon0 = 0x09F0; // A->D Copy
        custom.bltcon1 = 0;
        custom.bltapt = (APTR)getAssetData(dots_asset);
        custom.bltdpt = bar_overlay;
        custom.bltamod = 0;
        custom.bltdmod = 0;
        custom.bltsize = (256 << 6) | 20; // 256 lines, 20 words (320px)

        WaitBlit();
        custom.bltcon0 = 0x09F0;
        custom.bltcon1 = 0;  
        custom.bltapt = (APTR)getAssetData(dots_asset);
        custom.bltdpt = bar_overlay + (256 * 40);
        custom.bltamod = 0;
        custom.bltdmod = 0;
        custom.bltsize = (256 << 6) | 20;
    }

    // is it beneficial to move dots_asset back to fastmem, here?
    // maybe a CPU copy would be better then ?

    asset_moveToChip(dancer_asset);
    active_asset = dancer_asset;
    

    // Use override to display the bar_overlay on Bitplane 3 (Index 3)
    if (bar_overlay) {
        screen_overrideBitPlane(3, bar_overlay);
    }

    asset_moveToChip(dancer_asset);
    

    screen_disableDoubleBuffer();
    WaitTOF();

    screen_drawAssetPlane(active_asset, 0, 96, 20, 128, 185, 0, 0, 0);
    screen_drawAssetPlane(active_asset, 1, 96, 20, 128, 185, 0, 0, 0);
    screen_drawAssetPlane(active_asset, 2, 96, 20, 128, 185, 1, 0, 0);
    screen_drawAssetPlane(active_asset, 4, 96, 20, 128, 185, 2, 0, 0);
    palette_set(dancer_palette,32);

    screen_drawAssetPlane(dots_asset,3,0,0,320,256,0,0,0);

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
        if (scanline > 200){
            blend = 128;
        }
        if (scanline > 240){
            blend = 192;
        }
        darkCol = palette_blendColor(bgCol, 0x000, blend); 
        copperList[copperIndex++] = 0x0184; // COLOR02
        copperList[copperIndex++] = darkCol;
        copperList[copperIndex++] = 0x0194; // COLOR10
        copperList[copperIndex++] = darkCol;

        blend = 64;
        if (scanline > 200){
            blend = 40;
        }
        if (scanline > 240){
            blend = 16;
        }
        // Lighter version for COLOR08 (dots)
        lightCol = palette_blendColor(bgCol, 0xFFF, blend); // Blend with white
        
        copperList[copperIndex++] = 0x0190; // COLOR08
        copperList[copperIndex++] = lightCol;

        dist = (scanline > 249) ? (scanline - 249) : (249 - scanline);
        if (dist > 63) dist = 63;
        val = math_sin16(scanline);
        val = (val * dist) >> 6;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = val << 4;

        // Squash logic for Bitplane 2 
        if (scanline <= 249) {
             if (scanline < 147) {
                 src_y = 1; // Clamp top to safe area
             } else {
                 // Increased squash factor: 200/100 = 2.0
                 src_y = 205 - ((249 - scanline) * 200) / 100;
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

        if (modStep == 15 && step>4){
            loop = 0;
            modStep = 0;
        }

        if (modStep > 8){
            modStep = 0;
            // this is a beat-sync for re-used mod patterns later in the demo
        }


        if (modStep){
            step++;

            if (step>1){
                imageFrame++;
                if (imageFrame > 3){
                    imageFrame = 0;
                }
                yOffset = imageFrame*185;
                if (yOffset > 370) yOffset = 185;
                screen_drawAssetPlane(active_asset, 0, 96, 20, 128, 185, 0, 0, yOffset);
                screen_drawAssetPlane(active_asset, 1, 96, 20, 128, 185, 0, 0, yOffset);
                screen_drawAssetPlane(active_asset, 2, 96, 20, 128, 185, 1, 0, yOffset);
                screen_drawAssetPlane(active_asset, 4, 96, 20, 128, 185, 2, 0, yOffset);
            }

            if (step == 7){
                dancerWobble = 1;
            }

            if (step == 9){
                dancerWobble = 0;
            }

            if (step == 15){
                dancerWobble = 1;
            }

            if (step == 16){
                if (dancer_mirror_asset && dancer_mirror_asset != dancer_asset){
                    // TODO: if we have enough chip ram, maybe keep the dancer around for some more to re-use later?
                    asset_free(dancer_asset);
                    asset_moveToChip(dancer_mirror_asset);
                    active_asset = dancer_mirror_asset;
                }
            }

            if (step == 17){
                dancerWobble = 0;
            }

            if (step == 23){
                dancerWobble = 1;
            }

            if (step == 25){
                dancerWobble = 0;
            }

            if (step == 29){
                dancerWobble = 1;
            }

            
        }

        scrollY++;
        if (bar_overlay) {
             screen_overrideBitPlane(3, bar_overlay + (scrollY * 40));
        }

        copperIndex = extentionIndex;
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
            asset_free(dancer_asset);
            asset_free(dancer_mirror_asset);
            screen_overrideBitPlane(3, 0); // Reset override
        }
    }
}