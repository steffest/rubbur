#include "app.h"
#include "clib/graphics_protos.h"
#include "exec/types.h"
#include <clib/exec_protos.h>
#include <exec/memory.h>
#include <hardware/custom.h>

#define PLASMA_WIDTH 79
#define PLASMA_HEIGHT 60
#define PLASMA_BUFFER_SIZE (((PLASMA_WIDTH + 7) >> 3) * PLASMA_HEIGHT)

BYTE background_asset;
BYTE overlay_asset;
BYTE hands_asset;
extern BYTE dots_asset;

// Palettes
static UWORD plasma_palette[] = {0X000,0X012,0X133,0X555,0X000,0X356,0X577,0Xaaa};

// Palette for Part 2
static UWORD face_palette_part2[] = {
    0x000, 0x156, // face bitplane 1
    0x225, 0x033, // hands bitplane 1

    0X29a, 0X7dd, // face bitplane 2
    0x046, 0x067,

    0x488, 0x575, // hands bitplane 2
    0xaaa, 0x7cc,
    0x256, 0x556,
    0xccb, 0xbdd,

    //0x000, 0x378, 0x447, 0x255, 0X5cd, 0Xaff, 0x248, 0x289,
    0x000, 0x012, 0x001, 0x000, 0X056, 0X399, 0x002, 0x023,
    0x6aa, 0x797, 0xccc, 0x9ee, 0x478, 0x778, 0xeed, 0xdff,
};

static UWORD face_palette_part1[] = {
    //0x012, 0x156, // face bitplane 1
    0x000, 0x012, // face bitplane 1
    0x234, 0x378, // plasma
    //0X29a, 0X7dd, // face bitplane 2
    0x023, 0x034, // face bitplane 1
    0X4bc, 0X9ff, // plasma
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x012, 0x156, // overlay face bitplane 1
    0x345, 0x489, // plasma
    0X29a, 0X7dd, // overlay face bitplane 2
    0X5cd, 0Xbff, // plasma
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
};


extern struct Custom far custom;

static UBYTE *sine_table = NULL;
static UBYTE *sine_table2 = NULL;
static UBYTE *g_plasma_buffer = NULL;
static UWORD *wobblePtrs[256];

// Pre-calculated tables
static UBYTE yAddTable[PLASMA_HEIGHT];
static UBYTE ySubTable[PLASMA_HEIGHT];
static UBYTE ySin2Table[PLASMA_HEIGHT];

// ASM Render function
void __asm plasma_render_asm(
    register __a0 UBYTE *buffer, register __a1 UBYTE *sine_table,
    register __a2 UBYTE *sine_table2, register __a3 UBYTE *yAddTable,
    register __a4 UBYTE *ySubTable, register __a5 UBYTE *ySin2Table,
    register __d0 UBYTE t1, register __d1 UBYTE t2, register __d2 UBYTE t3);
    void plasma_preload() {
      loader_next();
    
      background_asset = asset_loadImage("data/face.planes", 240, 240, 2, MEMF_ANY);
      loader_next();
      overlay_asset = asset_loadImage("data/overlay_circle.planes", 320, 255, 1, MEMF_ANY);
      loader_next();
      hands_asset = asset_loadImage("data/hands.planes", 208, 594, 2, MEMF_ANY);
    }

void plasma_free() {
  if (g_plasma_buffer) {
    FreeMem(g_plasma_buffer, PLASMA_BUFFER_SIZE);
    g_plasma_buffer = NULL;
  }
  if (sine_table) {
    FreeMem(sine_table, 1024);
    sine_table = NULL;
  }
  if (sine_table2) {
    FreeMem(sine_table2, 1024);
    sine_table2 = NULL;
  }

  asset_free(background_asset);
  asset_free(overlay_asset);
  asset_free(hands_asset);
  asset_free(dots_asset);
}

void drawHandTop(UBYTE frame) {
  // area height: 128
  UBYTE x = 60;
  UBYTE y = 10;
  LONG sY = 0;
  UBYTE height = 107;

  if (frame == 2) {
    sY = 107;
    height = 86;
  }

  if (frame == 3) {
    y = 0;
    sY = 193;
    height = 128;
  }

  bitplane_clearRect(1, 48, 0, 208, 240);
  bitplane_clearRect(3, 48, 0, 208, 240);
  screen_drawAssetPlane(hands_asset, 1, x, y, 208, height, 0, 0, sY);
  screen_drawAssetPlane(hands_asset, 3, x, y, 208, height, 1, 0, sY);
}

void drawHandBottom(UBYTE frame) {
  // area height: 112
  UBYTE x = 48;
  UBYTE y = 128;
  LONG sY = 323;
  UBYTE height = 112;

  if (frame == 2) {
    sY = 438;
    height = 90;
  }

  if (frame == 3) {
    y = 138;
    sY = 529;
    height = 65;
  }

  screen_drawAssetPlane(hands_asset, 1, x, y, 208, height, 0, 0, sY);
  screen_drawAssetPlane(hands_asset, 3, x, y, 208, height, 1, 0, sY);
}


void appearHandBottom(SHORT width) {
  UBYTE rw = 176 - width;  


  screen_drawAssetPlane(hands_asset, 1, 48, 128, width, 112, 0, rw, 323);
  screen_drawAssetPlane(hands_asset, 3, 48, 128, width, 112, 1, rw, 323);
}

// targetBitplane: 1 for Part 1 (BP 2), 4 for Part 2 (BP 5)
void setup_copper_list(int targetBitplane) {
    UWORD copperIndex = copper_reset();
    UWORD* copperList = copper_getCopperList();
    
    int dy;
    int sy;
    
    int cIdx = copperIndex;
    int startLine = 0x2c;
    
    int targetHeight = 255;
    int srcHeight = PLASMA_HEIGHT;

    int lastSy = -1;
    ULONG addr;

    // Pre-fill wobblePtrs
    for (dy = 0; dy < 256; dy++) {
        wobblePtrs[dy] = NULL;
    }

    for (dy = 0; dy < targetHeight; dy++) {
      
      // Calculate the source line for this screen line
      sy = (dy * srcHeight) / targetHeight;
      
      // Wait for line
      copperList[cIdx++] = (((startLine + dy) & 0xff) << 8) | 1; 
      copperList[cIdx++] = 0xfffe; 

      // Always add horizontal wobble (BPLCON1)
      copperList[cIdx++] = BPLCON1;
      wobblePtrs[dy] = &copperList[cIdx];
      copperList[cIdx++] = 0;
   
      // If source line changed, update bitplane pointers
      if (sy != lastSy) {
          addr = (ULONG)(g_plasma_buffer + (sy * (PLASMA_WIDTH >> 3)));
          
          copperList[cIdx++] = BPL1PTH + (targetBitplane << 2);
          copperList[cIdx++] = (addr >> 16) & 0xffff;
          copperList[cIdx++] = BPL1PTL + (targetBitplane << 2);
          copperList[cIdx++] = addr & 0xffff;

          lastSy = sy;
      }
    }

    // End Copper List
    copperList[cIdx++] = 0xffff;
    copperList[cIdx++] = 0xfffe;
}


void plasma(void) {

  UWORD copperIndex = 0;
  UWORD framecount = 0;

  UBYTE loop = 1;
  UBYTE t1, t2, t3;
  UBYTE handIndex = 1;
  SHORT handYpos = 400;
  SHORT handWidth = 0;

  UBYTE modStep;  
  SHORT step = 0;

  SHORT blend = 254;
  BYTE blendStep = 2;

  struct DScreen *scr = screen_getScreen();


  // init plasma buffers
  int i;
  sine_table = AllocMem(1024, MEMF_ANY);
  sine_table2 = AllocMem(1024, MEMF_ANY);

  // Generate sine tables
  for (i = 0; i < 1024; i++) {
    sine_table[i] = math_sin32(i);
    sine_table2[i] = math_sin16(i);
  }

  // Pre-calculate Y tables
  for (i = 0; i < PLASMA_HEIGHT; i++) {
    yAddTable[i] = (UBYTE)i;
    ySubTable[i] = (UBYTE)(0 - i);
  }

  g_plasma_buffer = AllocMem(PLASMA_BUFFER_SIZE, MEMF_CHIP | MEMF_CLEAR); 
  
  if (!g_plasma_buffer) {
     if (sine_table) FreeMem(sine_table, 1024);
     if (sine_table2) FreeMem(sine_table2, 1024);
     return;
  }
  
  asset_moveToChip(background_asset);
  asset_moveToChip(overlay_asset);
  asset_moveToChip(hands_asset);

  if (dots_asset){
      asset_moveToChip(dots_asset);
  }

  screen_disableDoubleBuffer();
  screen_clear();
  //if (!g_plasma_buffer) return;

  // --- PART 1 SETUP ---
  palette_set(face_palette_part1, 32);

  palette_setColor(16, 0);
  palette_setColor(17, 0);
  palette_setColor(20, 0);
  palette_setColor(21, 0);

  // Copper: Wobble on BPL 2 (Index 1)
  setup_copper_list(1);
  copper_activate();

  // Face on BPL 1 & 3
  screen_drawAssetPlane(background_asset, 0, 30, 0, 240, 240, 0, 0, 0);
  screen_drawAssetPlane(background_asset, 2, 30, 0, 240, 240, 1, 0, 0);

  // Overlay on BPL 4
  screen_overrideBitPlane(3, getAssetData(overlay_asset));

  // Dots on BPL 5 (Use pointer override + scroll)
  // Initial frame defaults
  
  while (loop) {
    framecount++;

    modStep = mod_isStep();

    if (modStep == 15){
            loop = 0;
            modStep = 0;
        }


    if (modStep > 0) {
      step++;

      if (step == 16){
        // last step of part 1: show hand

        // swap bitplanes to make room for 2 hand bitplanes
        UBYTE* bpl2 = scr->canvas + (scr->planeSize * 1);
        UBYTE* bpl4 = scr->canvas + (scr->planeSize * 3);

        // draw circle overlay to background
        screen_eraseAssetPlane_simple(overlay_asset, 0);
        screen_eraseAssetPlane_simple(overlay_asset, 2);

        // 1. Reset Bitplane Pointers for Hands (BPL 2 & 4)
        screen_overrideBitPlane(1, bpl2 /*ptr*/);
        screen_overrideBitPlane(3, bpl4 /*ptr*/);
        
        palette_set(face_palette_part2, 32);
        
        // TODO: avoid complete copper reset?
        setup_copper_list(4);


        drawHandTop(3);

        blend = -100;
        blendStep = 10;

      }

      if (step > 16){
          handIndex = modStep;
          if (handIndex > 3) handIndex = 1;
          if (modStep == 5) handIndex = 3;
          if (modStep == 6) handIndex = 1;
          if (modStep == 7) handIndex = 3;
          if (modStep == 8) handIndex = 1;
          if (modStep == 9) handIndex = 1;
          if (modStep == 10) handIndex = 2;
          if (modStep == 11) handIndex = 1;
          drawHandTop(handIndex);

          if (handWidth < 176){
               handWidth += 8;
               appearHandBottom(handWidth);
          }else{
            handIndex = modStep;
            if (modStep == 5) handIndex = 2;
            if (modStep == 9) handIndex = 3;
            if (modStep == 7) handIndex = 3;
            if (modStep == 8) handIndex = 2;
            if (modStep == 10) handIndex = 3;
            if (modStep == 11) handIndex = 2;
            if (handIndex > 3) handIndex = 1;
            drawHandBottom(handIndex);

          }

      

          //drawHandBottom(handIndex);
      }
    }

    
    if (step < 16) {
        // Update Dots Scroll (BPL 5)
        ULONG baseAddr = (ULONG)getAssetData(dots_asset);
        // Note: dots asset is 320x256 -> 40 bytes/row
        int yOffset = (math_sin16(framecount << 1) - 8) * 40 * 2; // Approximate scroll
    
        screen_overrideBitPlane(4, (APTR)(baseAddr + (yOffset & 0xFFFFFFFC))); 

        // fade in part of the palette
      if (blend>0){
        blend -= 8;
        if (blend<0) blend=0;
        palette_setColor(16, palette_blendColor(face_palette_part1[16], 0, blend));
        palette_setColor(17, palette_blendColor(face_palette_part1[17], 0, blend));
        palette_setColor(20, palette_blendColor(face_palette_part1[20], 0, blend));
        palette_setColor(21, palette_blendColor(face_palette_part1[21], 0, blend));
      }

    }else{
      blend += blendStep;
      if (blend>210) blendStep = -10;
      if (blend<-100) blendStep = 10;

      if (blend<=0){
        palette_setColor(21, palette_blendColor(face_palette_part2[21], 0, -blend));
      }else{
        palette_setColor(21, palette_blendColor(face_palette_part2[21], 0xfff, blend));
      }
    }



    // --- SHARED: PLASMA UPDATE & WOBBLE ---
    
    // Update Wobble
    {
        int dy;
        UWORD shift;
        int sinIdx = (framecount << 2);
        
        for (dy = 0; dy < 255; dy++) {
             if (wobblePtrs[dy]) {
                 shift = math_sin16(sinIdx) & 0xF;
                 if (step < 16) {
                     *wobblePtrs[dy] = shift; // Part 1 
                 } else {
                     *wobblePtrs[dy] = shift << 4; // Part 2 (Plasma style)
                 }
                 sinIdx += 4;
            }
        }
    }

    // Render Plasma
    t1 = (UBYTE)(framecount);
    t2 = (UBYTE)(framecount >> 1);
    t3 = (UBYTE)(framecount << 2);
    plasma_render_asm(g_plasma_buffer, sine_table, sine_table2, yAddTable, ySubTable, ySin2Table, t1, t2, t3);

    
    WaitTOF();

    if (WRITE_DEBUG && isMouseDown() && framecount > 10) {
      loop = 0;
    }


  }

  copper_reset();
  WaitTOF();
  plasma_free();

  WaitTOF();
  screen_clear();
  WaitTOF();
}