#include <hardware/custom.h>
#include "app.h"
#include "framework/assetManager.h"
#include "framework/effect_screenzoom.h"
#include "parts/parts.h"
#include "stdio.h"

static UWORD dude2_palette[] = {0X000,0X335,0X431,0X742,0Xa64,0Xe40,0Xc96,0Xf74,0Xdb7,0Xfb6,0Xec9,0Xeda,0Xfda,0Xfec,0Xfff,0X210,
                                0X003,0X338,0X649,0Xc6c,0Xaad,0Xd46,0Xf9f,0Xf76,0Xdb4,0Xda7,0Xbc9,0Xedf,0Xfd3,0Xfe7,0Xffd,0X728};
extern struct Custom far custom;

extern BYTE dude_asset;
extern BYTE *bar_overlay;

void dude2_preload(){
    loader_next();
}

void dude2(void){
    UWORD frameCount = 0;
    WORD offset = 1;
    WORD pos = 1;
    WORD animFrame = 30;
    UBYTE loop = 1;
    UWORD i;
    UBYTE modStep;
    UBYTE step = 0;
    SHORT bendStrength = 0;
    BYTE bendDirection = 1;
    SHORT barY = 250;
    BYTE barDirection = -4;
    
    struct DScreen *screen;
    unsigned long addr;

    screen = screen_getScreen();
    addr = (APTR) (screen->canvas);
        
    copper_reset();
    screen_reset();
    screen_clear();
    
    generate_bar_overlay();
    asset_moveToChip(dude_asset);
    screen_disableDoubleBuffer();
    
    WaitTOF();
    screen_drawAsset(dude_asset, 60, 0, 192, 256, 0, 0);
    palette_set(dude2_palette,32);


    /* maxZoom=15, zoneSize=100. 
     * Note: maxZoom must be <= zoneSize / 2PI (approx 16) to avoid image folding/flipping
     */
    //build_effect_screenzoom(116, 180, 10);
    
    // full head
    set_screenzoom_planeCount(4);
    build_effect_screenzoom(10, 200, 40);
    setup_bend(10, 200);

    screen_overrideBitPlane(4, bar_overlay + (256*40));
    //screen_drawAssetPlane(UBYTE index, UBYTE dstPlane, WORD x, WORD y, USHORT width, USHORT height, UBYTE srcPlane, UWORD srcX, UWORD srcY)

    // update_screenzoom(0,-1);
    //update_screenzoom(20,1);

    while (loop){
        frameCount++;

        modStep = mod_isStep();


        if (modStep > 4 && modStep < 8){
            modStep = 0; // ignore in this section
        }
        
        if (modStep){
            step++;
            animFrame = 20;
            pos = modStep%2 == 0 ? 0 : 1;
            if (pos == 0){
                //update_screenzoom(70,1);
                update_screenzoom(0,1);
            }else{
                //update_screenzoom(124,-1);
                update_screenzoom(10,-1);
            }

            if (modStep == 4){
                if (step==4){
                    set_bend_strength(-16);
                }
                if (step==9){
                    set_bend_strength(16);
                }
            }

            if (modStep == 8){
                set_bend_strength(0);
                bendStrength = 0;
            }

            if (step == 6){
                build_effect_screenzoom(60, 182, 50);
            }

            if (step == 10){
                build_effect_screenzoom(10, 200, 40);
            }

             
        }
        

        if (step == 11){
                bendStrength -= 2;
                if (bendStrength < -20) bendStrength = -20;
                set_bend_strength(bendStrength);
        }

        if (step == 12){
                bendStrength += 2;
                if (bendStrength > 20) bendStrength = 20;
                set_bend_strength(bendStrength);
        }

        if (step == 13){
                bendStrength -= 2;
                if (bendStrength < -20) bendStrength = -20;
                set_bend_strength(bendStrength);
        }

         if (step == 14){
                bendStrength += 2;
                if (bendStrength > 20) bendStrength = 20;
                set_bend_strength(bendStrength);
        }

        if (step<26){
            animFrame--;
            if (animFrame < 0) animFrame = 0;
        }
        
        barY += barDirection;
        if (barY < 10) barDirection = 4;
        if (barY > 250) barDirection = -4;

        screen_overrideBitPlane(4, bar_overlay + (barY*40));
        
        effect_screenzoom(animFrame, 20); 
    
        WaitTOF();

        if (step == 30){
            loop = 0;
        }
      
        if (WRITE_DEBUG && isMouseDown() && frameCount > 10){
            loop = 0;
        }

        if (loop == 0){
           asset_free(dude_asset);
           //if (bar_overlay) FreeMem(bar_overlay, 320/8 * 512);
        }
    }

}