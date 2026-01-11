#include <exec/types.h>
#include "app.h"

#define maxColors 32

void palette_init(void){
    UBYTE i;
    for (i = 0; i < maxColors; i++) {
        copper_setColorValue(i, 0);
    }
}

void palette_set(UWORD *palette, UBYTE size){
    UBYTE i;
    for (i = 0; i < size; i++) {
        copper_setColorValue(i, palette[i]);
    }
}

void palette_fade(UWORD *palette, UBYTE size, UBYTE blend , UWORD targetColor){
    UBYTE i;
    for (i = 0; i < size; i++) {
        copper_setColorValue(i, palette_blendColor(palette[i], targetColor, blend));
    }
}

void palette_setBlack(UBYTE size){
    UBYTE i;
    for (i = 0; i < size; i++) {
        copper_setColorValue(i, 0x000);
    }
}

void palette_setColor(UBYTE index, UWORD color){
    copper_setColorValue(index, color);
}

// ranges from 0 to 255
UWORD palette_blendColor(UWORD color1, UWORD color2, UBYTE blend){
    UBYTE red1 = (color1 & 0xF00) >> 8;
    UBYTE green1 = (color1 & 0xF0) >> 4;
    UBYTE blue1 = color1 & 0xF;
    UBYTE red2 = (color2 & 0xF00) >> 8;
    UBYTE green2 = (color2 & 0xF0) >> 4;
    UBYTE blue2 = color2 & 0xF;
    UBYTE red = red1 + ((red2 - red1) * blend >> 8);
    UBYTE green = green1 + ((green2 - green1) * blend >> 8);
    UBYTE blue = blue1 + ((blue2 - blue1) * blend >> 8);
    return (UWORD)((red << 8) + (green << 4) + blue);
}

UWORD palette_blendRGB(UBYTE red1, UBYTE green1, UBYTE blue1, UBYTE red2, UBYTE green2, UBYTE blue2, UBYTE blend){
    UBYTE red = red1 + ((red2 - red1) * blend >> 8);
    UBYTE green = green1 + ((green2 - green1) * blend >> 8);
    UBYTE blue = blue1 + ((blue2 - blue1) * blend >> 8);
    return (UWORD)((red << 8) + (green << 4) + blue);
}

