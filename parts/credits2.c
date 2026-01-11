#include "app.h"
#include "exec/types.h"
#include <exec/memory.h>
#include <clib/exec_protos.h>

BYTE credits2_asset;
BYTE credits2_mid_asset;
BYTE text2_asset;

static UWORD credits2_palette[] = {0X000,0X211,0X321,0X332,0X541,0X974,0X552,0X662,0X355,0X577,0X9aa,0Xf73,0Xcb5,0Xdc6,0Xfc1,0Xfe6,0X100,0X200,0X301,0X511,0X921,0X941,0Xb21,0Xa52,0Xc51,0Xfa3,0Xfd4,0Xb72,0Xe90,0Xaa5,0Xe31,0X773};
static UWORD overlay2_colors[] = {0x6a9,0xaed,0xeff}; 

static UBYTE text2_shift[] = {4,2,1,0,0,1,2,3,3,4,4,5,5,6,6,6,
                             7,7,7,7,6,6,6,5,5,5,4,4,4,3,3,2,
                           2,2,1,1,1,2,2,3,3,4,4,5,6,7,8,9,
                            10,10,10,10,11,12,13,14,14,14,15,15,15,15,15,15,15} ;

extern struct Custom far custom;

void credits2_preload(){
    loader_next();
    credits2_asset = asset_loadImage("data/credits2.planes", 240, 176, 5, MEMF_ANY);
    loader_next();
    credits2_mid_asset = asset_loadImage("data/credits2_mid.planes", 240, 64, 4, MEMF_ANY);
    loader_next();
    text2_asset = asset_loadImage("data/text2.planes", 80, 200, 1, MEMF_ANY);
}

void credits2(){
    #define SCREEN_STARTLINE 44
    UBYTE loop = 1;
    UWORD frameCount = 0;
    UWORD extentionIndex = 0;
    UWORD copperIndex = 0;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline;
    UBYTE *text2_data;
    UBYTE *screen_bpl4;
    ULONG text_line_addr;
    ULONG screen_line_addr;
    UBYTE i = 0;
    USHORT w = 0;
    BYTE wobbleX = 0;
    SHORT scrollY = -30;
    SHORT textY = 0;
    UBYTE modStep = 0;

    extentionIndex = copper_reset();
    copperIndex = extentionIndex;
    screen_reset();
    screen_clear();
    screen_disableDoubleBuffer();


    palette_set(credits2_palette,32);
    asset_moveToChip(credits2_asset);
    asset_moveToChip(credits2_mid_asset);
    asset_moveToChip(text2_asset);

    screen_drawAsset(credits2_asset, 24, 0, 240, 18, 0, 0);
    screen_drawAsset(credits2_asset, 24, 82, 240, 158, 0, 18);

        // draw per line to compensate for text2_shift
        // TODO: bake this into the asset when curve is final;
        for (scanline = 0; scanline < 64; scanline++) {
            wobbleX = 24-text2_shift[scanline];
            w = 240;
            i = 0;
            if (wobbleX<=0){
                w = w + wobbleX;
                i = -wobbleX;
                wobbleX += 16;
            }
            screen_drawAsset(credits2_mid_asset, wobbleX, 18 + scanline, 240, 1, 0, scanline);
        }

        //screen_drawAsset(credits2_mid_asset, 24, 18, 240, 64, 0, 0);

        text2_data = AllocMem(40 * 200, MEMF_CHIP | MEMF_CLEAR);

        if (text2_data) {
            UBYTE *src = getAssetData(text2_asset);
            // Copy 80x200 (10 bytes wide) to 320x200 (40 bytes wide)
            // Source width: 80 pixels = 10 bytes = 5 words
            // Dest width: 320 pixels = 40 bytes = 20 words
            
            WaitBlit();
            custom.bltcon0 = 0x09F0; // A->D copy (USEA, USED, LF=F0)
            custom.bltcon1 = 0;
            
            custom.bltafwm = 0xFFFF;
            custom.bltalwm = 0xFFFF;

            custom.bltapt = (APTR)src;
            // Shift destination by 144 pixels (18 bytes)
            custom.bltdpt = (APTR)(text2_data + 18);

            // Modulos are in bytes
            custom.bltamod = 0;   // Source is 80px (10 bytes/5 words) wide, no skip needed
            custom.bltdmod = 30;  // Dest is 320px (40 bytes) - 10 bytes written = 30 bytes skip

            // Size: Height 200, Width 5 words
            custom.bltsize = (200 << 6) | 5;
            WaitBlit();
            asset_free(text2_asset);
        } 


        // setup copper
        for (scanline = 0; scanline < 64; scanline++) {
            
            // wait for line
            copperList[copperIndex++] = ((scanline + SCREEN_STARTLINE + 18) << 8) + 7;
            copperList[copperIndex++] = 0xfffe;

            if (scanline == 0){
                // set overlay colors

                // monitor colors
                for (i=4; i<8; i++){
                    copperList[copperIndex++] = 0x180 + (i+16 << 1);
                    copperList[copperIndex++] = credits2_palette[i];
                }

                // screen colors
                for (i=8; i<11; i++){
                    copperList[copperIndex++] = 0x180 + (i+16 << 1);
                    copperList[copperIndex++] = overlay2_colors[i-8];
                }
            }

            text_line_addr = (ULONG)text2_data + (scanline * 40);

            wobbleX = text2_shift[scanline];
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

        // reset line 82

        copperList[copperIndex++] = ((82 + SCREEN_STARTLINE) << 8) + 7;
        copperList[copperIndex++] = 0xfffe;

        // reset overlay colors
        for (i=4; i<11; i++){
            copperList[copperIndex++] = 0x180 + (i+16 << 1);
            copperList[copperIndex++] = credits2_palette[i+16];
        }
        

        screen_bpl4 = screen_getBitplaneAddress(4);
        screen_line_addr = (ULONG)screen_bpl4 + (82 * 40);

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

        
    for (scanline = 0; scanline < 64; scanline++) {
                // skip Wait
                copperIndex += 2;

                if (scanline == 0){
                    // skip colors
                    copperIndex += 14;
                }

                //set bitplane pointer
                textY = scrollY + scanline - (math_sin16(scanline << 1));
                if (textY<0) textY = 0;
                if (textY>200) textY = 0;

                text_line_addr = (ULONG)text2_data + (textY * 40);
                if (text2_shift[scanline] > 15){
                    text_line_addr --;
                }


                copperList[copperIndex++] = BPL5PTH;
                copperList[copperIndex++] = (text_line_addr >> 16) & 0xFFFF;
                copperList[copperIndex++] = BPL5PTL;
                copperList[copperIndex++] = text_line_addr & 0xFFFF;

                // skip 
                copperIndex += 2;
            }

        
        WaitTOF();


        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
            asset_free(credits2_asset);
            asset_free(credits2_mid_asset);

            if (text2_data) {
                FreeMem(text2_data, 40 * 200);
            }
        }
    }
}