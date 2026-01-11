#include <hardware/custom.h>
#include <graphics/gfxbase.h>
#include "app.h"

#define BG_COLOR				(0x000)
#define CENTER_LINE				(150)

// copper instruction macros
#define COP_MOVE(addr, data) addr, data
#define COP_WAIT(loc, mask) loc, mask

// playfield control
// single playfield, 5 bitplanes (32 colors)
#define BPLCON0_VALUE (0x5200)

#define BPLCON1_VALUE (0x0000)

// http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0092.html
// bits 14,13,12 define the number of bitplanes

// 0x0048: playfield 2 has priority over sprites (bit 6)
// 0x0020: default mode ?
// see http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0159.html
#define BPLCON2_VALUE (0x0020)

#define DIWSTRT_VALUE      0x2c81
#define DIWSTOP_VALUE_PAL  0x2cc1
#define DIWSTOP_VALUE_NTSC 0xf4c1

// Data fetch
#define DDFSTRT_VALUE      0x0038
#define DDFSTOP_VALUE      0x00d0

// copper list indexes
#define COPLIST_IDX_SPR0_PTH_VALUE (3)
#define COPLIST_IDX_DIWSTOP_VALUE (COPLIST_IDX_SPR0_PTH_VALUE + 32 + 6)
#define COPLIST_IDX_BPLCON1_VALUE (COPLIST_IDX_DIWSTOP_VALUE + 4)
#define COPLIST_IDX_BPLCON2_VALUE (COPLIST_IDX_DIWSTOP_VALUE + 6)
#define COPLIST_IDX_BPL1MOD_VALUE (COPLIST_IDX_DIWSTOP_VALUE + 8)
#define COPLIST_IDX_BPL2MOD_VALUE (COPLIST_IDX_BPL1MOD_VALUE + 2)
#define COPLIST_IDX_COLOR00_VALUE (COPLIST_IDX_BPL2MOD_VALUE + 2)
#define COPLIST_IDX_BPL1PTH_VALUE (COPLIST_IDX_COLOR00_VALUE + 64)

#define COPLIST_EXTENTION (COPLIST_IDX_BPL1PTH_VALUE + 19)
#define COPLIST_IDX_COLOR_VALUE (COPLIST_IDX_BPL1PTH_VALUE + 22)

// TODO: are these recalculated every time?

static UWORD __chip copperList[7000];

extern struct Custom far custom;
extern struct GfxBase *GfxBase;

UWORD palFixPositon = 0;
UWORD extentionIndex = 0;

void copper_init(){    
    UWORD i = 0;
    UWORD index = 0;
    UWORD value = 0;
    UWORD max = sizeof(copperList) / sizeof(UWORD);
    
    //for (i = 0; i < max; i++) copperList[i] = 0;

    copperList[0] = 0x1fc; // FMODE slow fetch mode, AGA compatibility (no idea what this is but all the fancy code examples have it)
    copperList[1] = 0;

    // sprites first
    index = 2;
    value = SPR0PTH;
    for (i = 0; i < 16; i++){
        copperList[index] = value;
        index += 2;
        value += 2;
    }
    copperList[index++] = DDFSTRT;
    copperList[index++] = DDFSTRT_VALUE;
    copperList[index++] = DDFSTOP;  
    copperList[index++] = DDFSTOP_VALUE;
    copperList[index++] = DIWSTRT;
    copperList[index++] = DIWSTRT_VALUE;
    copperList[index++] = DIWSTOP;
    copperList[index++] = DIWSTOP_VALUE_PAL;
    copperList[index++] = BPLCON0;
    copperList[index++] = BPLCON0_VALUE;
    copperList[index++] = BPLCON1;
    copperList[index++] = BPLCON1_VALUE;
    copperList[index++] = BPLCON2;
    copperList[index++] = BPLCON2_VALUE;
    copperList[index++] = BPL1MOD;
    copperList[index++] = 0;
    copperList[index++] = BPL2MOD;
    copperList[index++] = 0;

    // set up the display colors
    for (i = 0; i < 32; i++){
        copperList[index++] = COLOR00 + (i << 1);
        copperList[index++] = 0;
    }

    // bitplane pointers
    value = BPL1PTH;
    for (i = 0; i < (SCREENDEPTH*2); i++){
        copperList[index++] = value;
        copperList[index++] = 0;
        value += 2;
    }

    // PAL fix
    copperList[index++] = 0xffdf;
    copperList[index++] = 0xfffe;

    // end copperlist
    copperList[index++] = 0xffff;
    copperList[index++] = 0xfffe;

    if (index-4 != COPLIST_EXTENTION){
        printf("ERROR: index != COPLIST_EXTENTION\n");
    }


}

UWORD copper_reset(void){
  copperList[COPLIST_EXTENTION] = 0xffdf;
  copperList[COPLIST_EXTENTION+1] = 0xfffe;
  // end copperlist
  copperList[COPLIST_EXTENTION+2] = 0xffff;
  copperList[COPLIST_EXTENTION+3] = 0xfffe;

  extentionIndex = COPLIST_EXTENTION;
  return extentionIndex;
}

void copper_activate(){
   custom.cop1lc = copperList;
}

void copper_restore(){
    custom.cop1lc = (unsigned long) ((struct GfxBase *) GfxBase)->copinit;
}

void copper_setColorValue(unsigned short index, unsigned short value){
    copperList[COPLIST_IDX_COLOR00_VALUE + (index << 1)] = value;
}

void copper_setBitPlane(unsigned short index, unsigned short value){
    //3+32+6+8+2+2+64
    // = 117
    copperList[COPLIST_IDX_BPL1PTH_VALUE + index] = value;
    //copperList[117 + index] = value;
}

void copper_setSpritePointer(unsigned short *sprite, char spriteIndex){
    copperList[COPLIST_IDX_SPR0_PTH_VALUE + (spriteIndex << 2)] = (((ULONG) sprite) >> 16) & 0xffff;
    copperList[COPLIST_IDX_SPR0_PTH_VALUE + (spriteIndex << 2) + 2] = ((ULONG) sprite) & 0xffff;
}

void copper_setBPLCON1(UWORD value){
    copperList[COPLIST_IDX_BPLCON1_VALUE] = value;
}

// used to position playfield in front/behind sprites
/*
0x00-0x07: Sprite in front of all playfields
0x08-0x0F: Sprite behind PF1 (priority 1)
0x10-0x17: Sprite behind PF1 (priority 2)
0x18-0x1F: Sprite behind PF1 (priority 3)
0x20-0x27: Sprite behind PF1 (priority 4) ← behind all bitplanes
*/
void copper_setBPLCON2(UWORD value){
    copperList[COPLIST_IDX_BPLCON2_VALUE] = value;
}

void copper_setValue(unsigned short index, unsigned short value){
    copperList[COPLIST_IDX_COLOR_VALUE + index] = value & 0xffff;
}

void copper_setExtention(unsigned short index, unsigned short value){
    copperList[COPLIST_EXTENTION + index] = value & 0xffff;
}

UWORD copper_start(void){
    extentionIndex = COPLIST_EXTENTION;
    return extentionIndex;
}

UWORD copper_getExtentionIndex(){
    return extentionIndex;
}

void copper_setExtentionIndex(UWORD index){
    extentionIndex = COPLIST_EXTENTION + index;
}

UWORD copper_waitForLine(UWORD line){
    copperList[extentionIndex++] = line*256 + 7;
    copperList[extentionIndex++] = 0xfffe;
    return extentionIndex-2;
}

UWORD copper_setColor(UWORD index, UWORD color){
    copperList[extentionIndex++] = 0x180 + (index << 1);
    copperList[extentionIndex++] = color;
    return extentionIndex-2;
}

UWORD copper_end(){
    copperList[extentionIndex++] = 0xffff;
    copperList[extentionIndex++] = 0xfffe;
    return extentionIndex-2;
}

UWORD* copper_getCopperList(){
    return copperList;
}


