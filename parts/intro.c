#include "app.h"
#include "framework/assetManager.h"
#include "framework/effect_screenzoom.h"

BYTE intro_asset;
BYTE title_asset;
BYTE title_background_asset;
BYTE cycle_palettes_asset;

//static UWORD intro_palette[] = {0X000,0X030,0X042,0X051,0X253,0X063,0X264,0X675,0X194,0X3a5,0X2b6,0X6c7,0X7e9,0Xafb,0Xcfd,0Xfff};
static UWORD intro_palette[] = {0X000,0X021,0X042,0X444,0X050,0X053,0X064,0X096,0X495,0X6b7,0Xbbb,0X0f0,0Xced,0Xafb,0Xefe,0Xfff};
static UWORD title_palette[] = {0X000,0X200,0X112,0X412,0X224,0X232,0X626,0X722,0X346,0X643,0X354,0X457,0X747,0X55e,0Xc39,0Xb51,0Xb54,0X27b,0X674,0X57a,0Xa69,0Xe65,0Xb95,0X4ad,0X6b5,0Xd9b,0Xabe,0Xfa5,0Xac6,0Xfd7,0Xeed,0X4ff};

    // 32 frames of 32 colors
    static UWORD* cycle_palettes;
    
    void intro_preload(){
        loader_next();
        intro_asset = asset_loadImage("data/rubberband2.planes", 80, 64, 4, MEMF_CHIP);
        loader_next();
        title_asset = asset_loadImage("data/rubbur.planes", 416, 119, 5, MEMF_ANY);
        loader_next();
        title_background_asset = asset_loadImage("data/rubburback.planes", 288, 197, 5, MEMF_CHIP);
        loader_next();
        cycle_palettes_asset = asset_loadFile("data/intro.pal", MEMF_ANY);
    }
    
    void intro(){
        UBYTE loop = 1;
        UWORD frameCount = 0;
        
        WORD strength = 32;
        UWORD blend = 0;
        WORD lensY = 0;
        WORD elastic_amp = 0;
        UBYTE wobbleStep = 20;
        UBYTE step = 0;

        UBYTE wobbleOffset[] = {0,15,1,14,2,13,3,12,4,11,5,10,6,9,7,8,7};


        screen_disableDoubleBuffer();
        screen_reset();
        screen_clear();
        WaitTOF();
        palette_set(intro_palette,16);
    
        
        copper_setBPLCON1(7<<4 | 7);
        
        cycle_palettes = (UWORD*)getAssetData(cycle_palettes_asset);
        asset_moveToChip(intro_asset);
        
        build_effect_screenzoom(100, 190,100); 
        effect_screenzoom(32, 32); 
        set_screenzoom_top(0);
        screen_drawAsset(intro_asset, 120, 124, 80, 64, 0, 0);
        asset_free(intro_asset);

        
        
       
        while (loop){
            frameCount++;
            
            if (mod_isStep()){
                step++;
                if (step == 2){
                    update_screenzoom(20,1);
                }

                if (step == 4){
                    update_screenzoom(70,-1);
                }

                 if (step == 5){
                    disable_screenzoom();
                    wobbleStep = 0;
                    set_screenzoom_wobble(1);
                }

                 if (step == 6){
                    strength = 0;
                    update_screenzoom(10,1);
                    enable_screenzoom();
                }

                if (step == 11){
                    disable_screenzoom();
                    wobbleStep = 0;
                }

                if (step == 12){
                    wobbleStep = 0;
                }

                 if (step == 13){
                    strength = 0;
                    update_screenzoom(100,1);
                    enable_screenzoom();
                }

                 if (step == 15){
                    palette_setBlack(16);
                    set_screenzoom_top(0);
                    set_screenzoom_wobble(0);
                    build_effect_screenzoom(0, 150,100); 
                    screen_drawAsset(title_background_asset, 16, 4, 288, 197, 0, 0);
                    effect_screenzoom(1, 1); 
                    blend = 255;
                    strength = 32;
                }

                 if (step == 16){
                    asset_moveToChip(title_asset);
                    asset_free(title_background_asset);
                 }

                if (step == 17){
                    effect_screenzoom(0, 1); 
                    palette_set(title_palette,32);
                }

                if (step == 20){
                    palette_set(title_palette,32);
                }


                if (step == 21){
                    palette_set(title_palette,32);
                    disable_screenzoom();
                    wobbleStep = 0;
                    screen_drawAsset(title_asset, 16, 47, 96, 119, 0, 0);
                }

                if (step == 22){
                    build_effect_screenzoom(20, 180,20); 
                    setup_lens_profile(100, 10);
                    update_lens_position(80);
                }

                if (step == 25){
                    wobbleStep = 0;
                    screen_drawAsset(title_asset, 16+64, 47, 64, 119, 96, 0);
                    build_effect_screenzoom(20, 180,40); 
                    disable_screenzoom();
                }

                if (step == 26){
                     enable_screenzoom();
                     strength = 0;
                }

                if (step == 27){
                   strength = 5;
                }

                if (step == 29){
                    disable_screenzoom();
                    wobbleStep = 0;
                    screen_drawAsset(title_asset, 16+64+32, 47, 64, 119, 96+64, 0);
                }

                if (step == 30){
                    update_screenzoom(20, -1);
                     enable_screenzoom();
                     strength = 0;
                }

                if (step == 31){
                   strength = 5;
                }

                if (step == 33){
                    wobbleStep = 0;
                    screen_drawAsset(title_asset, 16+64+32+48, 47, 64, 119, 96+64+64, 0);
                    build_effect_screenzoom(20, 180,0); 
                    setup_bend(20, 180);
                    disable_screenzoom();
                }

                if (step == 34){
                    enable_screenzoom();
                    strength = 0;
                }

                if (step == 35){
                    strength = 16;
                }

                if (step == 37){
                    disable_screenzoom();
                    wobbleStep = 0;
                    screen_drawAsset(title_asset, 16+64+32+48+32, 47, 64, 119, 96+64+64+64, 0);
                    set_bend_strength(0);
                }

                if (step == 38){
                    enable_screenzoom();
                    strength = 0;
                }


                if (step == 39){
                    strength = 16;
                }

                if (step == 41){
                    wobbleStep = 0;
                    screen_drawAsset(title_asset, 16+64+32+48+32+48, 47, 64, 119, 96+64+64+64+64, 0);
                    build_effect_screenzoom(0, 80,0); 
                    setup_bend(0, 160);
                    elastic_amp = 1;
                    disable_screenzoom();
                }

                if (step > 44 && step<56){
                    enable_screenzoom();
                    strength = 8 * elastic_amp;
                    if (strength < 0) strength = -16;
                    elastic_amp = 1-elastic_amp;
                }

                if (step == 56){
                    build_effect_screenzoom(30, 180,400);
                    update_screenzoom(30,1);
                    strength = 0; 
                    blend = 0;
                }
           
            }

            if (wobbleStep<17){
                copper_setBPLCON1(7<<4 | wobbleOffset[wobbleStep]);
                wobbleStep++;
            }


            if (step == 1){         
                strength -= 2;
                lensY -= math_sin16(frameCount << 1) >> 2;
                if (strength < 0) strength = 0;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
            }

            if (step == 2){         
                lensY++;
                strength ++;
                if (strength > 32) strength = 32;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
                
            }

            if (step == 3){         
                strength--;
                lensY++;
                if (strength < 0) strength = 0;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
            }

             if (step == 4){         
                lensY++;
                strength ++;
                if (strength > 32) strength = 32;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
            }

            if (step == 6){         
                strength ++;
                if (strength > 32) strength = 32;
                effect_screenzoom(strength, 32); 
            }

            if (step == 8){         
                lensY-=2;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
            }


            if (step == 9){         
                strength --;
                if (strength < 0) strength = 0;
                effect_screenzoom(strength, 32); 
            }

            if (step == 10){         
                lensY+=2;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
            }

            if (step == 13){         
                lensY +=4;
                strength ++;
                if (strength > 32) strength = 32;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 
            }

            if (step == 14){         
                lensY +=8;
                set_screenzoom_top(lensY);
                effect_screenzoom(strength, 32); 

            }

            if (step == 15 || step == 16){          
                strength--;
                if (strength < 0) strength = 0;
                effect_screenzoom(strength, 32); 
                blend -= 8;
                if (blend < 0) blend = 0;
                palette_fade(intro_palette, 16, blend, 0x000);

            }

            if (step > 16 && step<20){
                if ((frameCount & 1) == 0) {
                    UBYTE palIdx = (frameCount >> 1) % 32;
                    palette_set(&cycle_palettes[palIdx * 32], 32);
                }
            }


            if (step == 22 || step == 23 || step == 24){
                lensY = 80 + (math_sin(frameCount * 3) >> 2);
                update_lens_position(lensY);
                strength = math_sin(frameCount * 4) >> 3; // +- 32
                set_bend_strength(strength >> 1);
                effect_screenzoom(strength, 32); 
            }

            if (step == 26 || step == 30){
                strength++;
                if (strength > 4) strength = 4;
                effect_screenzoom(strength, 4); 
            }

            if (step == 27 || step == 31){
                strength--;
                if (strength < 0) strength = 0;
                effect_screenzoom(strength, 4); 
            }

            if (step == 34){
                strength++;
                if (strength > 16) strength = 16;
                set_bend_strength(strength);
                effect_screenzoom(1,1); 
            }

            if (step == 35){
                strength--;
                if (strength < 0) strength = 0;
                set_bend_strength(strength);
                effect_screenzoom(1,1); 
            }

            if (step == 38){
                strength++;
                if (strength > 16) strength = 16;
                set_bend_strength(-strength);
                effect_screenzoom(1,1); 
            }

            if (step == 39){
                strength--;
                if (strength < 0) strength = 0;
                set_bend_strength(-strength);
                effect_screenzoom(1,1); 
            }


            if (step > 40  && step < 56){
                if ((frameCount & 1) == 0) {
                    UBYTE palIdx = (frameCount >> 1) % 32;
                    palette_set(&cycle_palettes[palIdx * 32], 32);
                }
            }

            if (step > 44 && step < 56){
                strength += (elastic_amp);
                set_bend_strength(strength);
                effect_screenzoom(1, 1); 
            }

            if (step > 55){
                strength ++;
                if (strength > 32) strength = 32;
                effect_screenzoom(strength, 32); 

                blend += 8;
                palette_fade(title_palette, 32, blend, 0x000);
            }


    
            if (step == 60){
                screen_clear();
                loop = 0;
            }
    
            if (WRITE_DEBUG && isMouseDown()){
                loop = 0;
            }
    
    
            WaitTOF();
        }
    
        asset_free(intro_asset);
        asset_free(title_asset);
        asset_free(title_background_asset);
        asset_free(cycle_palettes_asset);
        copper_setBPLCON1(0);
    }