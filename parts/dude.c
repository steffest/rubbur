#include <exec/memory.h>
#include <hardware/custom.h>
#include <clib/timer_protos.h>
#include <clib/exec_protos.h>
#include "app.h"
#include "exec/types.h"

static UWORD dude_palette[] = {0X000,0X335,0X431,0X742,0Xa64,0Xe40,0Xc96,0Xf74,0Xdb7,0Xfb6,0Xec9,0Xeda,0Xfda,0Xfec,0Xfff,0X210,0X421,0X335,0X431,0X742,0Xa64,0Xe40,0Xc96,0Xf74,0Xdb7,0Xfb6,0Xec9,0Xeda,0Xfda,0Xfec,0Xfff,0X210};
static UWORD dude2_palette[] = {0X000,0X335,0X505,0X145,0X956,0Xb3a,0Xb50,0Xc75,0Xda9,0Xfb7,0Xeb9,0Xfaa,0Xfd9,0Xfdc,0Xfff,0X210};
extern struct Custom far custom;

BYTE dude_asset;
BYTE *bar_overlay;

static UBYTE local_sinTable[] = {
    8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 11, 11,
    11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13,
    13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 10, 10,
    10, 10, 10, 10, 9,  9,  9,  9,  9,  8,  8,  8,  8,  8,  8,  7,  7,  7,  7,
    7,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  3,
    3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  3,
    3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  6,
    6,  6,  6,  6,  7,  7,  7,  7,  7};

static UWORD y_offset_table[512];

void generate_bar_overlay(VOID){
    if (bar_overlay) return;
    bar_overlay = AllocMem(320/8 * 512, MEMF_CHIP | MEMF_CLEAR);
    // Fill mask with white on a 32 range
    if(bar_overlay){
         WaitBlit();
         custom.bltcon0 = 0x01FF; // Use D, LF=0xFF (All 1s)
         custom.bltcon1 = 0;
         custom.bltdpt = bar_overlay + (256 * 40);
         custom.bltdmod = 0;
         custom.bltsize = (22 << 6) | 20; // 22 lines, 20 words (40 bytes)
    }else{
       if (WRITE_DEBUG) printf("Failed to allocate memory for bar overlay mask\n");
    }
}


void dude_preload(){
    int i;
    loader_next();
    dude_asset = asset_loadImage("data/dude1.planes", 192, 256, 4, MEMF_ANY);
    
    // Pre-calculate Y offsets (y * 40)
    for(i=0; i<512; i++) {
        y_offset_table[i] = i * 40;
    }

}

void dude(void){
    UWORD frameCount = 0;
    UBYTE loop = 1;
    UBYTE startY = 50;
    UBYTE sideWobbleStartY = 108;
    UBYTE sideWobbleEndY = 210;
    UBYTE endWobbleStartY = 247;
    UBYTE endWobbleEndY = 255;
    UBYTE dampingShift = 1;
    UBYTE stretchY = 0;
    UWORD offset = 0;
    UWORD step = 0;
    UBYTE sideStep = 0;
    UBYTE modStep = 0;
    UBYTE fadeStep = 0;
    SHORT slideY = 0;


    USHORT scanline;
    UBYTE line_width_bytes = 40;
    UWORD extentionIndex = 0;
    UBYTE overlayScrollX;

    UBYTE downCounter = 0;
    UBYTE beatIndex = 0;

    UWORD copperIndex = 0;
    UWORD value = 0;
    WORD sinValue;
    register UWORD *cp;

    BYTE beatOffsets[4][8] = {
        {5, 4, 4, 3, 3, 2, 1, 0},
         {4, 4, 3, 3, 3, 2, 2, 2},
         {2, 2, 2, 3, 3, 3, 4, 4},
        {0, 1, 2, 3, 4, 5, 6, 7},
    };

    BYTE beatOffsets2[4][8] = {
         {10, 9, 7, 6, 4, 3, 1, 0},
         {7, 6, 5, 4, 3, 2, 1, 0},
         {3, 3, 2, 2, 2, 1, 1, 0},
         {0, 0,0, 0,0, 0,0, 0},
    };


    static UWORD __chip sprite0[] = {
 0x0000, 0x0000, 
  0x1f9f, 0xe060, 
  0xcf3f, 0x30c0, 
  0xfeef, 0x0110, 
  0xb0f3, 0x4f0c, 
  0xa1fd, 0x5e02, 
  0xdbfc, 0x2403, 
  0x01fe, 0xfe01, 
  0x0324, 0xfcdb, 
  0x1f28, 0xe0d7, 
  0x3f3c, 0xc0c3, 
  0x7bff, 0x8400, 
  0x7ffd, 0x8002, 
  0x8ffb, 0x7004, 
  0x3be0, 0xc41f, 
  0x9791, 0x686e, 
  0x89de, 0x7621, 
  0x0927, 0xf6d8, 
  0x0460, 0xfb9f, 
  0x5040, 0xafbf, 
  0xc431, 0x3bce, 
  0xe33e, 0x1cc1, 
  0x41af, 0xbe50, 
  0xe1e9, 0x1e16, 
  0x607c, 0x9f83, 
  0x601b, 0x9fe4, 
  0x3816, 0xc7e9, 
  0x3a1c, 0xc5e3, 
  0x1837, 0xe7c8, 
  0x1f6b, 0xe094, 
  0x0b6d, 0xf492, 
  0xddbb, 0x2244, 
  0xb6db, 0x4924, 
  0x3ede, 0xc121, 
  0x6743, 0x98bc, 
  0x4fdf, 0xb020, 
  0x3edf, 0xc120, 
  0x17cf, 0xe830, 
  0xefe7, 0x1018, 
  0xe6fb, 0x1904, 
  0x78fd, 0x8702, 
  0xfcbc, 0x0343, 
  0xf7de, 0x0821, 
  0x7467, 0x8b98, 
  0xff63, 0x009c, 
  0xfed9, 0x0126, 
  0x79ee, 0x8611, 
  0x3877, 0xc788, 
  0x883b, 0x77c4, 
  0xc03f, 0x3fc0, 
  0xf00b, 0x0ff4, 
  0xf957, 0x06a8, 
  0xf83a, 0x07c5, 
  0x1c16, 0xe3e9, 
  0xe229, 0x1dd6, 
  0xfe9e, 0x0161, 
  0x7e26, 0x81d9, 
  0xf9ef, 0x0610, 
  0xdc6d, 0x2392, 
  0x97b7, 0x6848, 
  0xccbd, 0x3342, 
  0xe52c, 0x1ad3, 
  0x813f, 0x7ec0, 
  0x081f, 0xf7e0, 
  0x080f, 0xfff0, 
  0x040f, 0xfff0, 
  0x0b8f, 0xf7f0, 
  0x0307, 0xfff8, 
  0x05eb, 0xfff4, 
  0x1eff, 0xfff0, 
  0x7f44, 0xfffb, 
  0x1f00, 0xffff, 
  0xcf9b, 0xffff, 
  0x7fd4, 0xffb4, 
  0xffd4, 0xffb4, 
  0x7ff8, 0xffb8, 
  0x7ff0, 0xffb0, 
  0x81a0, 0xffe0, 
  0x67c0, 0xffc0, 
  0x77c0, 0xffc0, 
  0xf680, 0xfe80, 
  0xee00, 0xfe00, 
  0x7e00, 0xfe00, 
  0xb600, 0xfe00, 
  0x0000, 0x0000
};

    UBYTE beatY;
    ULONG plane1Base, plane2Base, plane3Base, plane4Base;


    struct DScreen *screen;
    unsigned long addr;

    UWORD* copperList = copper_getCopperList();
    extentionIndex = copper_reset();
    copperIndex = extentionIndex;
    //screen_clear();
    palette_set(dude_palette,32);

    generate_bar_overlay();

    screen = screen_getScreen();
    addr = (APTR) (screen->canvas);
    
    plane1Base = (ULONG)screen->canvas;
    plane2Base = plane1Base + screen->planeSize;
    plane3Base = plane2Base + screen->planeSize;
    plane4Base = plane3Base + screen->planeSize;

    // Top Wobble area
    for (scanline = startY; scanline < sideWobbleStartY; scanline++) {
        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0 ;

        copperList[copperIndex++] = BPL1PTH;
        copperList[copperIndex++] = (addr >> 16) & 0xffff;;
        copperList[copperIndex++] = BPL1PTL;
        copperList[copperIndex++] = addr & 0xffff;

        addr = (APTR) (screen->canvas + screen->planeSize);
        copperList[copperIndex++] = BPL2PTH;
        copperList[copperIndex++] = (addr >> 16) & 0xffff;;
        copperList[copperIndex++] = BPL2PTL;
        copperList[copperIndex++] = addr & 0xffff;

        addr = (APTR) (screen->canvas + (2 * screen->planeSize));
        copperList[copperIndex++] = BPL3PTH;
        copperList[copperIndex++] = (addr >> 16) & 0xffff;;
        copperList[copperIndex++] = BPL3PTL;
        copperList[copperIndex++] = addr & 0xffff;

        addr = (APTR) (screen->canvas + (3 * screen->planeSize));
        copperList[copperIndex++] = BPL4PTH;
        copperList[copperIndex++] = (addr >> 16) & 0xffff;;
        copperList[copperIndex++] = BPL4PTL;
        copperList[copperIndex++] = addr & 0xffff;
    }

    // Side Wobble area
    stretchY = 0;
    for (scanline = sideWobbleStartY; scanline < sideWobbleEndY; scanline++) {
        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0 ;

        if (stretchY<8){
            
            // set Bitplane pointers for stretched area;
            copperList[copperIndex++] = BPL1PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL1PTL;
            copperList[copperIndex++] = addr & 0xffff;

            addr = (APTR) (screen->canvas + screen->planeSize);
            copperList[copperIndex++] = BPL2PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL2PTL;
            copperList[copperIndex++] = addr & 0xffff;

            addr = (APTR) (screen->canvas + (2 * screen->planeSize));
            copperList[copperIndex++] = BPL3PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL3PTL;
            copperList[copperIndex++] = addr & 0xffff;

            addr = (APTR) (screen->canvas + (3 * screen->planeSize));
            copperList[copperIndex++] = BPL4PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL4PTL;
            copperList[copperIndex++] = addr & 0xffff;

            stretchY++;
        }



        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 115;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0 ;


        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 115 + 40;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0 ;
    }

     // reset horizontal wobble to normal for rest of screen
    copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
    copperList[copperIndex++] = 0xfffe;
    copperList[copperIndex++] = BPLCON1;
    copperList[copperIndex++] = 0 ;

    //printf("CopperIndex: %d\n", copperIndex);


    // End Wobble area
    for (scanline = endWobbleStartY; scanline < endWobbleEndY; scanline++) {
        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
        copperList[copperIndex++] = 0xfffe;

        
        // set Bitplane pointers for stretched area;
            copperList[copperIndex++] = BPL1PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL1PTL;
            copperList[copperIndex++] = addr & 0xffff;

            addr = (APTR) (screen->canvas + screen->planeSize);
            copperList[copperIndex++] = BPL2PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL2PTL;
            copperList[copperIndex++] = addr & 0xffff;

            addr = (APTR) (screen->canvas + (2 * screen->planeSize));
            copperList[copperIndex++] = BPL3PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL3PTL;
            copperList[copperIndex++] = addr & 0xffff;

            addr = (APTR) (screen->canvas + (3 * screen->planeSize));
            copperList[copperIndex++] = BPL4PTH;
            copperList[copperIndex++] = (addr >> 16) & 0xffff;;
            copperList[copperIndex++] = BPL4PTL;
            copperList[copperIndex++] = addr & 0xffff;

    }


    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;


    printf("CopperIndex Max: %d\n", copperIndex);
    sprite_init();
    screen_reset();
    // set sprite colors
    // sprite 0 and 1
    copper_setColorValue(17, 0x431);
    copper_setColorValue(18, 0x210);
    copper_setColorValue(19, 0xd40);

    sprite_attach(sprite0, 0);
    sprite_setPosition(sprite0, 320, 108, 83);
    sprites_on();
    
    // Set playfield priority over sprites in single playfield mode
    // Bit 6 (0x40) = PF2PRI enables playfield priority
    // Bits 0-2 = priority level (higher = more in front)
    copper_setBPLCON2(0x44);
   

    if (WRITE_DEBUG && isMouseDown()) return;

    asset_moveToChip(dude_asset);
    screen_disableDoubleBuffer();
    

    WaitTOF();
    screen_drawAsset(dude_asset, 60, 0, 192, 256, 0, 0);

    
    while (loop){
        frameCount++;

        modStep = mod_isStep();

        if (modStep == 15){
            loop = 0;
            modStep = 0;
        }


        if (modStep){
            sideStep = 0;
        
            if (modStep<5){
                step++;
                downCounter = 0;
                if (modStep == 4){
                    sideStep = 2;
                }

                if (step == 5){
                    palette_set(dude2_palette, 16);
                }

                if (step == 9){
                    palette_set(dude_palette, 16);
                }

                 if (step == 16){
                    fadeStep = 0;
                    slideY = 255;
                    
                    sprite_detach(0);
                    sprites_off();

                    palette_setColor(16, 0x000);
                }
            }
            
        }else{
            downCounter++;
        }

         if (step == 16){
            fadeStep += 16;
            slideY -= 16;
            if (slideY < 0) {
                slideY = 0;
            }
        
            palette_fade(dude2_palette,16,fadeStep,0x000);  
            if (bar_overlay) {
                screen_overrideBitPlane(4, bar_overlay + (slideY*40));
            }

            if (slideY == 0){
                palette_setBlack(32);
            }

         }

        cp = &copperList[extentionIndex];

        // top wobble
        dampingShift = 1; // Divide by 2
        beatIndex = downCounter >> 3;
        for (scanline = startY; scanline < sideWobbleStartY; scanline++) {
            cp += 2; // skip wait
            
            // Use shifts instead of division for damping
            if ((scanline & 7) == 0 && dampingShift < 3) dampingShift++;

            // horizontal wobble
            cp++; // skip BPLCON1
            sinValue = local_sinTable[(frameCount + (scanline << 2)) & 0xFF];
            value = sinValue;
            *cp++ = (value << 4) | value;


            // vertical wobble - adjust bitplane pointers
            // Use lookup table instead of multiplication
            // offset = (scanline - 44 + (sinValue / damping)) * line_width_bytes;
            offset = y_offset_table[scanline - 44 + (sinValue >> dampingShift)];

            addr = plane1Base + offset;
            cp++; // skip register
            *cp++ = addr >> 16;
            cp++; // skip register
            *cp++ = addr;

            addr = plane2Base + offset;
            cp++; // skip register
            *cp++ = addr >> 16;
            cp++; // skip register
            *cp++ = addr;

            addr = plane3Base + offset;
            cp++; // skip register
            *cp++ = addr >> 16;
            cp++; // skip register
            *cp++ = addr;

            addr = plane4Base + offset;
            cp++; // skip register    
            *cp++ = addr >> 16;
            cp++; // skip register        
            *cp++ = addr;    
        }

        // side wobble
        dampingShift = 0;
        stretchY = 0;
        cp += 3;
        for (scanline = sideWobbleStartY; scanline < sideWobbleEndY; scanline++) {
            overlayScrollX = local_sinTable[(scanline + (frameCount << 1)) & 0xFF] >> 1;
            *cp++ = (overlayScrollX << 4) | overlayScrollX;

            if (stretchY < 8){
                beatY = beatOffsets[beatIndex][stretchY];
                
                // vertical wobble - adjust bitplane pointers
                // offset = (scanline - 54 + beatY) * line_width_bytes;
                offset = y_offset_table[scanline - 54 + beatY];
                offset += sideStep;

                addr = plane1Base + offset;
                cp++; // skip register
                *cp++ = addr >> 16;
                cp++; // skip register
                *cp++ = addr;

                addr = plane2Base + offset;
                cp++; // skip register
                *cp++ = addr >> 16;
                cp++; // skip register
                *cp++ = addr;

                addr = plane3Base + offset;
                cp++; // skip register
                *cp++ = addr >> 16;
                cp++; // skip register
                *cp++ = addr;

                addr = plane4Base + offset;
                cp++; // skip register    
                *cp++ = addr >> 16;
                cp++; // skip register        
                *cp++ = addr;    

                stretchY++;
            } 



            cp += 3;

        
             *cp++ = 0;
            cp += 3; // Fixed from 4

            if ((scanline & 31) == 0 && dampingShift < 2) dampingShift++;

             overlayScrollX = local_sinTable[(scanline + (frameCount << 2)) & 0xFF] & 0x0F;
             overlayScrollX = overlayScrollX >> dampingShift;
             *cp++ = (overlayScrollX << 4) | overlayScrollX;
            cp += 3; // Fixed from 4
        }   

        // reset horizontal wobble
        cp++;

        // end wobble
        stretchY = 0;
        for (scanline = endWobbleStartY; scanline < endWobbleEndY; scanline++) {

            // vertical wobble - adjust bitplane pointers
            beatY = beatOffsets2[beatIndex][stretchY];
            // offset = (scanline - 40 - beatY) * line_width_bytes;
            offset = y_offset_table[scanline - 40 - beatY];

            cp += 2; // skip wait


            addr = plane1Base + offset;
            cp++; // skip register
            *cp++ = addr >> 16;
            cp++; // skip register
            *cp++ = addr;

            addr = plane2Base + offset;
            cp++; // skip register
            *cp++ = addr >> 16;
            cp++; // skip register
            *cp++ = addr;

            addr = plane3Base + offset;
            cp++; // skip register
            *cp++ = addr >> 16;
            cp++; // skip register
            *cp++ = addr;

            addr = plane4Base + offset;
            cp++; // skip register    
            *cp++ = addr >> 16;
            cp++; // skip register        
            *cp++ = addr;    

            stretchY++;
        }   

        
        WaitTOF();

    
        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
           //asset_free(dude_asset);
           //if(bar_overlay) FreeMem(bar_overlay, 320/8 * 512);
           screen_overrideBitPlane(4, 0); 
           palette_setBlack(32);
        }
    }

}