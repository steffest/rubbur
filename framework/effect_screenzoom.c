#include "app.h"

#define SCREEN_STARTLINE 44 /* hmmm.... shouldn't this be the same as DDFSTRT_VALUE? apparently not. */
#define MAX_BULGE_LINES 256  /* Maximum possible zoom zone size */

static UWORD copperIndices[256];  
static UBYTE *bitplanePointers[SCREENDEPTH];
static WORD zoomYStart = -1;
static WORD zoomYEnd = -1;
static WORD bytesPerLine = 40;
static WORD zoomLevel = -1;
static WORD bulgeCurve[MAX_BULGE_LINES];  
static WORD bulgeZoneSize = 0;  /* Actual size of the bulge zone */
ULONG plane1Base, plane2Base, plane3Base, plane4Base, plane5Base;
static UBYTE planeCount = 5;
static BYTE bulgeDirection = 1;
static BYTE isEnabled = 1;
static WORD globalYOffset = 0; /* Vertical offset for the entire screen */
static UBYTE globalWobble = 0;

void set_screenzoom_top(WORD yOffset) {
    globalYOffset = yOffset;
}

void set_screenzoom_planeCount(UBYTE count) {
    planeCount = count;
}

/* Forward declaration */
void precalc_bulge_curve(WORD maxZoom, WORD zoneSize);

static UBYTE bendCurve[256];
static WORD bendYStart = 0;
static WORD bendYEnd = 0;
static BYTE bendStrength = 0;

void reset_effect_screenzoom(void) {
    WORD i;
    
    zoomYStart = -1;
    zoomYEnd = -1;
    zoomLevel = 1; 
    bulgeZoneSize = 0;
    bulgeDirection = 1;
    isEnabled = 1;
    globalYOffset = 0;
    globalWobble = 0;
    
    bendYStart = 0;
    bendYEnd = 0;
    bendStrength = 0;
    
    for(i=0; i<MAX_BULGE_LINES; i++) bulgeCurve[i] = 0;
    for(i=0; i<256; i++) bendCurve[i] = 0;
}

void setup_bend(WORD start, WORD end) {
    WORD i;
    WORD height = end - start;
    WORD angle;
    WORD val;

    bendYStart = start;
    bendYEnd = end;

    for (i = 0; i < height; i++) {
        // Map 0..height to 0..128 (0..PI)
        angle = (i * 128) / (height - 1);
        val = math_sin(angle);
        
        // math_sin returns 128..255..128 for 0..128 input
        // we want 0..127..0
        if (val >= 128) {
            val -= 128;
        } else {
            val = 0; // Should not happen
        }
        
        if (i < 256) {
            bendCurve[i] = (UBYTE)val;
        }
    }
}

void set_bend_strength(BYTE strength) {
    bendStrength = strength;
}

void set_screenzoom_wobble(UBYTE wobble) {
    globalWobble = wobble;
}

// LENS MODE HELPERS

static WORD lensHeight = 0;

// Setup a fixed-height lens profile
// This pre-calculates both the Zoom (Bulge) and Bend curves for a specific height (diameter)
void setup_lens_profile(WORD height, WORD maxZoom) {
    lensHeight = height;

    precalc_bulge_curve(maxZoom, height);

    // not sure about this ... should be always set the ben as well ?
    setup_bend(0, height); 
    zoomLevel = maxZoom;
}

// Move the lens to a specific center Y
void update_lens_position(WORD centerY) {
    WORD offset = lensHeight >> 1;
    WORD start = centerY - offset;
    WORD end = start + lensHeight;

    zoomYStart = start;
    zoomYEnd = end;
    
    bendYStart = start;
    bendYEnd = end;
}

/* Pre-calculate the bulge curve for the zoom zone */
void precalc_bulge_curve(WORD maxZoom, WORD zoneSize) {
    WORD i;
    WORD angle;
    WORD sinValue;
    
    bulgeZoneSize = zoneSize;
    
    /* Calculate bulge for each line in the zoom zone
     * The curve is a full sine wave: 0 -> +max -> 0 -> -max -> 0
     * math_sin(0..256) returns a full wave (0..2PI).
     * This creates the "S" shape: Squish-Stretch-Squish.
     */
    for (i = 0; i < zoneSize; i++) {
        /* Map position to angle 0->256 (0°->360°) */
        angle = (i * 128) / (zoneSize - 1);
        
        sinValue = math_sin(angle);
        sinValue = sinValue - 128;

        /* Store pre-calculated bulge offset for maximum zoom */
        bulgeCurve[i] = ((maxZoom * sinValue) >> 7);  /* Divide by 128 */
    }
}

void enable_screenzoom(){
    UWORD copperIndex;
    UWORD *copperList;
    isEnabled = 1;
    copperList = copper_getCopperList();
    copperIndex = copper_start();
    copperList[copperIndex++] = (1<<8) + 7;
    copperList[copperIndex++] = 0xfffe;
}

void disable_screenzoom(){
    UWORD copperIndex;
    UWORD *copperList;
    isEnabled = 0;
    copperList = copper_getCopperList();
    copperIndex = copper_start();
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;
}


void build_effect_screenzoom(WORD yStart, WORD yEnd, WORD zoom) {
    UWORD copperIndex;
    struct DScreen *screen;
    UWORD rasterLine;
    UBYTE plane;
    UWORD *copperList;
    UWORD byteOffset;
    unsigned long addr;
    UWORD index;

    precalc_bulge_curve(zoom, yEnd - yStart);  

    screen = screen_getScreen();
    copperList = copper_getCopperList();
    copperIndex = copper_start();

    bitplanePointers[0] = screen->canvas;
    bitplanePointers[1] = screen->canvas + screen->planeSize;
    bitplanePointers[2] = screen->canvas + screen->planeSize * 2;
    bitplanePointers[3] = screen->canvas + screen->planeSize * 3;
    bitplanePointers[4] = screen->canvas + screen->planeSize * 4;


    plane1Base = (ULONG)bitplanePointers[0];        
    plane2Base = (ULONG)bitplanePointers[1];
    plane3Base = (ULONG)bitplanePointers[2];
    plane4Base = (ULONG)bitplanePointers[3];
    plane5Base = (ULONG)bitplanePointers[4];

    zoomYStart = yStart;
    zoomYEnd = yEnd;
    zoomLevel = zoom;

    copperList[copperIndex++] = (1<<8) + 7;
    copperList[copperIndex++] = 0xfffe;

    // setup a copper list that sets the bitplane pointer for each line on the entire screen.
    for (rasterLine = 0; rasterLine < 210; rasterLine++) {
        // 210: lets avoid the PAL fix
        
        /* Wait for this scanline */

        index = (rasterLine + SCREEN_STARTLINE);

        /*if (index>=256){
            //palFixPositon = index;
            copperList[copperIndex++] = 0xffdf;
            copperList[copperIndex++] = 0xfffe;
        }*/

        copperList[copperIndex++] = (index<<8) + 7;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0;
        
        byteOffset = rasterLine * bytesPerLine;
        
        for (plane = 0; plane < planeCount; plane++) {
                addr = (APTR) (bitplanePointers[plane] + byteOffset);    
                copperList[copperIndex++] = BPL1PTH + (plane * 4); 
                copperList[copperIndex++] = (addr >> 16) & 0xffff;

                copperList[copperIndex++] = BPL1PTL + (plane * 4); 
                copperList[copperIndex++] = addr & 0xffff;
        }
    }

    // end of list
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;
    
}

void update_screenzoom(UWORD yStart, BYTE direction){
    UBYTE height = zoomYEnd - zoomYStart;
    zoomYStart = yStart;
    zoomYEnd = yStart + height;  
    bulgeDirection = direction;
}


/* Update the bitplane pointers in the pre-built copper list */
void effect_screenzoom(UWORD currentFrame, WORD totalFrames) {
    
    UWORD currentZoom;
    UWORD centerY;
    UWORD halfZone;
    WORD relativeY;
    WORD bulgeOffset;
    WORD sourceLine;
    UWORD rasterLine;
    UWORD activeLine;
    unsigned long addr;
    UBYTE plane;
    UWORD byteOffset;
    register UWORD *cp;  /* Register pointer for speed! */
    struct DScreen *screen;
    UWORD extentionIndex;
    WORD curveIndex;
    WORD zoomScale;  /* Pre-calculate zoom scaling factor */
    UBYTE overlayScrollX;
    WORD overlayOffsetWords;
    WORD signedOffset;
    
    // note: should we enforce totalFrames to be a power of 2?
    if (currentFrame >= totalFrames) {
        currentZoom = zoomLevel;
    } else {
        currentZoom = (currentFrame * zoomLevel) / totalFrames;
    }

    screen = screen_getScreen();
    extentionIndex = copper_start();
    
 
    centerY = (zoomYStart + zoomYEnd) >> 1;
    halfZone = (zoomYEnd - zoomYStart) >> 1;
    
    if (zoomLevel == 0) {
        zoomScale = 0;
    } else {
        zoomScale = (currentZoom << 8) / zoomLevel;
    }
    
    cp = &copper_getCopperList()[extentionIndex];

    /* Skip WAIT instruction */
    cp += 2;

    for (rasterLine = 0; rasterLine < 210; rasterLine++) {

        // Check if we need to process this line:
        // 1. Zoom is active on this line
        // 2. Bend is active on this line
        // 3. Global Y Offset is set (affects all lines)
        // Use totalFrames = 0 to force a full screen update
        if (globalYOffset == 0 && totalFrames > 0 &&
            ((WORD)rasterLine < zoomYStart ||(WORD)rasterLine >= zoomYEnd) && 
            ((WORD)rasterLine < bendYStart || (WORD)rasterLine >= bendYEnd)) {
            cp += 4 + (planeCount << 2); // 2(Wait) + 2(BPLCON) + 4 words per plane
            continue;
        }

        activeLine = rasterLine;
        
        // --- Zoom / Bulge Calculation ---
        if ((WORD)rasterLine >= zoomYStart && (WORD)rasterLine < zoomYEnd) {
            curveIndex = activeLine - zoomYStart;
            
            /* Get bulge offset from pre-calculated curve and scale it */
            bulgeOffset = (bulgeCurve[curveIndex] * zoomScale) >> 8;
            
            /* Calculate source line with clamping */
            sourceLine = (WORD)activeLine + bulgeDirection * bulgeOffset - globalYOffset;
        } else {
            sourceLine = (WORD)activeLine - globalYOffset;
        }

        if (sourceLine < 0) sourceLine = 0;
        if (sourceLine >= SCREENHEIGHT) sourceLine = SCREENHEIGHT - 1;


        // --- Bend / Horizontal Scroll Calculation ---
        if (bendStrength != 0 && (WORD)rasterLine >= bendYStart && (WORD)rasterLine < bendYEnd) {
             // Use pre-calculated bell curve
            UBYTE curveVal = bendCurve[rasterLine - bendYStart];
            // Scale by strength. 
            WORD xScroll = (curveVal * bendStrength) >> 6;
            
            if (xScroll >= 0) {
                // Right Shift
                overlayScrollX = xScroll & 0x0F;
                overlayOffsetWords = xScroll >> 4;
            } else {
                // Left Shift
                // xScroll is negative (e.g. -17)

                WORD absScroll = -xScroll;
                WORD words = (absScroll + 15) >> 4; // Ceil(absScroll/16)
                
                overlayOffsetWords = -words;
                overlayScrollX = (words << 4) - absScroll;
            }

        } else {
             overlayScrollX = 0;
             if (globalWobble != 0) {
                 overlayScrollX = math_sin16(rasterLine<<2);
             }
             overlayOffsetWords = 0;
        }

        

        
        /* Skip WAIT instruction */
        cp += 2;
        
        // Update BPLCON1
        cp++; // Skip
        *cp++ = (overlayScrollX << 4) | overlayScrollX;

        
        // Use bit shift for multiplication by 40 (32 + 8)
        // TODO: is this faster in general ?
        signedOffset = (sourceLine << 5) + (sourceLine << 3);
        
        // Adjust for horizontal bend (subtract offset in words * 2 bytes)
        signedOffset -= (overlayOffsetWords << 1);

        // Clamp to buffer limits to prevent corruption
        if (signedOffset < 0) signedOffset = 0;
        if (signedOffset > 10200) signedOffset = 10200;

        byteOffset = (UWORD)signedOffset;

        /* Update all 5 bitplanes using pointer arithmetic - NO array indexing! */
        // kind of weird that array lookups are slow on Amiga
        // something to remember ...

        addr = plane1Base + byteOffset;
        cp++;  /* Skip register */
        *cp++ = addr >> 16;
        cp++;  /* Skip register */
        *cp++ = addr;
        
        addr = plane2Base + byteOffset;
        cp++;
        *cp++ = addr >> 16;
        cp++;
        *cp++ = addr;
        
        addr = plane3Base + byteOffset;
        cp++;
        *cp++ = addr >> 16;
        cp++;
        *cp++ = addr;
        
        addr = plane4Base + byteOffset;
        cp++;
        *cp++ = addr >> 16;
        cp++;
        *cp++ = addr;

        if (planeCount >= 5){
            addr = plane5Base + byteOffset;
            cp++;
            *cp++ = addr >> 16;
            cp++;
            *cp++ = addr;
        }
    }
}
