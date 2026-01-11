#ifndef ZOOM_H
    #define ZOOM_H

    #include <exec/types.h>


/* Build the copper list structure for zoom effect */
void build_effect_screenzoom(WORD yStart, WORD yEnd, WORD zoom);

void update_screenzoom(UWORD yStart, BYTE direction);

/* Update zoom effect each frame */
void effect_screenzoom(UWORD currentFrame, WORD totalFrames);

void enable_screenzoom();
void disable_screenzoom();
void set_screenzoom_top(WORD yOffset);
void set_screenzoom_planeCount(UBYTE count);

void reset_effect_screenzoom(void);
void setup_bend(WORD start, WORD end);
void set_bend_strength(BYTE strength);

void set_screenzoom_wobble(UBYTE wobble);

void setup_lens_profile(WORD height, WORD maxZoom);
void update_lens_position(WORD centerY);

#endif
