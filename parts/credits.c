#include "app.h"
#include "exec/types.h"
#include <exec/memory.h>
#include <clib/exec_protos.h>

BYTE credits_asset;
extern BYTE credits_mid_asset;
BYTE text_asset;

extern UWORD credits_palette[];

static UWORD overlay_colors[] = {0x123,0x233,0x344,0x566} ;
static UWORD overlay_colors_dark[] = {0x012,0x122,0x233,0x345};

static UBYTE text_shift[] = {31,30,30,29,28,27,26,25,24,23,23,22,22,21,21,20,
                             20,19,19,18,18,17,17,18,18,19,19,20,20,20,19,19,
                            18,17,16,15,14,13,13,12,12,11,11,10,10,9,9,10,
                             10,11,11,12,12,13,13,13,14,14,14,15,15,16,16,17,17} ;


extern struct Custom far custom;

void credits_preload(){
    loader_next();
    credits_asset = asset_loadImage("data/credits1.planes", 304, 165, 5, MEMF_ANY);
    loader_next();
    text_asset = asset_loadImage("data/text1.planes", 80, 137, 1, MEMF_ANY);
}

void credits(){
    #define SCREEN_STARTLINE 44
    UBYTE loop = 1;
    UWORD frameCount = 0;
    UWORD extentionIndex = 0;
    UWORD copperIndex = 0;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline;
    UBYTE *text_data;
    UBYTE *text2_data;
    UBYTE *screen_bpl4;
    ULONG text_line_addr;
    ULONG screen_line_addr;
    UBYTE i = 0;
    USHORT w = 0;
    BYTE wobbleX = 0;
    SHORT scrollY = -60;
    SHORT textY = 0;
    UBYTE modStep = 0;

    extentionIndex = copper_reset();
    copperIndex = extentionIndex;
    screen_reset();
    screen_clear();
    screen_disableDoubleBuffer();

    palette_set(credits_palette,32);
        asset_moveToChip(credits_asset);
        asset_moveToChip(credits_mid_asset);
        asset_moveToChip(text_asset);

        screen_drawAsset(credits_asset, 16, 10, 304, 42, 0, 0);
        screen_drawAsset(credits_asset, 16, 115, 304, 123, 0, 42);

        // draw per line to compensate for text_shift
        // TODO: bake this into the asset when curve is final;
        for (scanline = 0; scanline < 63; scanline++) {
            wobbleX = 16-text_shift[scanline];
            w = 304;
            i = 0;
            if (wobbleX<=0){
                w = w + wobbleX;
                i = -wobbleX;
                wobbleX += 16;
            }
            screen_drawAsset(credits_mid_asset, wobbleX, 52 + scanline, w, 1, i, scanline);
        }
        //screen_drawAsset(credits_mid_asset, 16, 52, 304, 63, 0, 0);


        // draw text on separate off screen plane
        text_data = AllocMem(40 * 200, MEMF_CHIP | MEMF_CLEAR);
        if (text_data) {
            UBYTE *src = getAssetData(text_asset);
            // Copy 80x137 (10 bytes wide) to 320x200 (40 bytes wide)
            // Source width: 80 pixels = 10 bytes = 5 words
            // Dest width: 320 pixels = 40 bytes = 20 words
            
            WaitBlit();
            custom.bltcon0 = 0x09F0; // A->D copy (USEA, USED, LF=F0)
            custom.bltcon1 = 0;
            
            custom.bltafwm = 0xFFFF;
            custom.bltalwm = 0xFFFF;

            custom.bltapt = (APTR) src;
            // Shift destination by 32 pixels (4 bytes) and 1 row down (40 bytes)
            custom.bltdpt = (APTR)(text_data + 4 + 40);

            // Modulos are in bytes
            custom.bltamod = 0;   // Source is 80px (10 bytes/5 words) wide, no skip needed
            custom.bltdmod = 30;  // Dest is 320px (40 bytes) - 10 bytes written = 30 bytes skip

            // Size: Height 137, Width 5 words
            custom.bltsize = (137 << 6) | 5;
            WaitBlit();
            asset_free(text_asset);
        } 

        // setup copper
        for (scanline = 96; scanline < 159; scanline++) {
            // wait for line
            copperList[copperIndex++] = (scanline << 8) + 7;
            copperList[copperIndex++] = 0xfffe;


            if (scanline == 96){
                // set overlay colors

                // monitor colors
                for (i=12; i<16; i++){
                    copperList[copperIndex++] = 0x180 + (i+16 << 1);
                    copperList[copperIndex++] = credits_palette[i];
                }

                // screen colors
                for (i=4; i<8; i++){
                    copperList[copperIndex++] = 0x180 + (i+16 << 1);
                    copperList[copperIndex++] = overlay_colors[i-4];
                }
            }


            if (scanline == 96 + 10){
                // screen colors
                for (i=4; i<8; i++){
                    copperList[copperIndex++] = 0x180 + (i+16 << 1);
                    copperList[copperIndex++] = overlay_colors_dark[i-4];
                }
            }

            if (scanline == 96 + 40){
                // screen colors
                for (i=4; i<8; i++){
                    copperList[copperIndex++] = 0x180 + (i+16 << 1);
                    copperList[copperIndex++] = overlay_colors[i-4];
                }
            }



            //text_line_addr = (ULONG)getAssetData(text_asset) + (scanline - 96) * 38;
            text_line_addr = (ULONG)text_data + (scanline - 96) * 40;

            wobbleX = text_shift[scanline-96];
            if (wobbleX>15){
                wobbleX -= 16;
            }
        
            //set bitplane pointer
            copperList[copperIndex++] = BPL5PTH;
            copperList[copperIndex++] = (text_line_addr >> 16) & 0xFFFF;
            copperList[copperIndex++] = BPL5PTL;
            copperList[copperIndex++] = text_line_addr & 0xFFFF;

            // set bplcon1
            copperList[copperIndex++] = BPLCON1;
            copperList[copperIndex++] = wobbleX << 4 | wobbleX ;
            
        }

        // reset line 159

        copperList[copperIndex++] = (159 << 8) + 7;
        copperList[copperIndex++] = 0xfffe;

        // reset colors
        for (i=12; i<16; i++){
            copperList[copperIndex++] = 0x180 + (i+16 << 1);
            copperList[copperIndex++] = credits_palette[i+16];
        }
        for (i=4; i<8; i++){
            copperList[copperIndex++] = 0x180 + (i+16 << 1);
            copperList[copperIndex++] = credits_palette[i+16];
        }

        screen_bpl4 = screen_getBitplaneAddress(4);
        screen_line_addr = (ULONG)screen_bpl4 + (159 - 44) * 40;

        copperList[copperIndex++] = BPL5PTH;
        copperList[copperIndex++] = screen_line_addr >> 16;
        copperList[copperIndex++] = BPL5PTL;
        copperList[copperIndex++] = screen_line_addr & 0xFFFF;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0;

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
         
        if (frameCount % 3 == 0){
            scrollY++;
        }

        copperIndex = extentionIndex;

       for (scanline = 0; scanline < 63; scanline++) {
                // skip Wait
                copperIndex += 2;

                // skip colors
                if (scanline == 0){
                    copperIndex += 16;
                }

                if (scanline == 10){
                    copperIndex += 8;
                }

                if (scanline == 40){
                    copperIndex += 8;
                }

                //set bitplane pointer
                textY = scrollY + scanline - (math_sin16(scanline << 2) >> 1);
                if (textY<0) textY = 0;
                if (textY>200) textY = 0;

                text_line_addr = (ULONG)text_data + (textY * 40);
                if (text_shift[scanline] > 15){
                    text_line_addr --;
                }


                copperIndex++;
                copperList[copperIndex++] = (text_line_addr >> 16) & 0xFFFF;
                copperIndex++;
                copperList[copperIndex++] = text_line_addr & 0xFFFF;

                // skip Wait
                copperIndex += 2;
            }



    
        WaitTOF();

     
        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
            asset_free(credits_asset);
            asset_free(credits_mid_asset);
            if (text_data) {
                FreeMem(text_data, 40 * 200);
            }
        }
    }
}