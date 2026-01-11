#include <exec/types.h>
void palette_init(void);
void palette_set(UWORD *palette, UBYTE size);
void palette_setBlack(UBYTE size);
void palette_fade(UWORD *palette, UBYTE size, UBYTE blend , UWORD targetColor);
void palette_setColor(UBYTE index, UWORD color);
void palette_setBlendColor(UBYTE index, UWORD color, UBYTE blend);
UWORD palette_blendColor(UWORD color1, UWORD color2, UBYTE blend);
UWORD palette_blendRGB(UBYTE red1, UBYTE green1, UBYTE blue1, UBYTE red2, UBYTE green2, UBYTE blue2, UBYTE blend);