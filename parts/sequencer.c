#include "app.h"

BYTE credits_mid_asset;
BYTE credits2b_mid_asset;

UWORD credits_palette[] = {0X000,0X023,0X233,0X443,0X044,0X365,0X477,0X6a9,0Xfc6,0Xc74,0X753,0Xea5,0X675,0Xbb9,0Xcca,0Xfec,0X011,0X012,0X122,0X022,0X034,0X232,0X443,0X055,0X353,0X067,0X254,0Xa64,0X1a9,0X996,0Xaa9,0Xedb};

UWORD credits_palette2[] = {0X000,0X211,0X302,0X313,0X412,0X502,0X523,0X433,0X623,0X702,0X912,0X734,0X646,0Xa23,0X653,0X944,0X945,0Xb43,0Xa55,0X765,0X579,0Xc65,0Xa95,0Xe73,0Xd96,0Xba5,0X7bb,0Xfa3,0Xdb6,0Xfb4,0Xcdc,0Xed6};

void sequencer_preload(){
    loader_next();
    credits_mid_asset = asset_loadImage("data/credits1_mid.planes", 304, 63, 4, MEMF_ANY);
    loader_next();
    credits2b_mid_asset = asset_loadImage("data/credits2b.planes", 304, 63, 5, MEMF_ANY);
}

void sequencer(){
    UBYTE loop = 1;
    UWORD frameCount = 0;
    UBYTE modStep;
    UBYTE step = 0;
    UBYTE endCommands = 0;
    WORD x = 0;
    UBYTE y = 20;
    UBYTE ignore = 0;
    UBYTE scanline = 0;

    UWORD extentionIndex = 0;
    UWORD copperIndex = 0;
    UWORD* copperList = copper_getCopperList();

    UBYTE wobbleState = 0;
    UBYTE val = 0;
    UBYTE wobbleStrength = 2;


    UBYTE yPos[] = {20,36,20,36,36,20,36,52,20,20,36,20,36,20,36,20,36,36,20,36};


    extentionIndex = copper_reset();
    screen_reset();
    screen_clear();
    copperIndex = extentionIndex;

    asset_moveToChip(credits_mid_asset);
    asset_moveToChip(credits2b_mid_asset);
    screen_disableDoubleBuffer();
    WaitTOF();

    palette_set(credits_palette2,32);
     palette_set(credits_palette,16);

    screen_drawAsset(credits_mid_asset, x,y,16,63,0,0); 


    // setup copperlist
    for (scanline = 50; scanline < 255; scanline++) {

        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0;

        if (scanline == 170){
            // set first 16 colors to credits_palette2
            for (x = 0; x < 16; x++){
                copperList[copperIndex++] = 0x0180 + (x << 1);
                copperList[copperIndex++] = credits_palette2[x];
            }
        }

    }
    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;

    x=0;


    while (loop){
        frameCount++;

        modStep = mod_isStep();

        if (modStep == 15){
            endCommands++;

            if (endCommands>1){
                loop = 0;
                modStep = 0;
            }
        }

        if (modStep > 0) {
            step++;
            ignore = 0;

            if (step == 12) ignore = 1;
            if (step == 38) ignore = 1;
            

            if (!ignore){
                if (step>1 && step<22){
                    x+=16;
                    if (x<300){
                        screen_drawAsset(credits_mid_asset, x,yPos[x/16],16,63,x,0); 
                    }
                }

                if (step == 22){
                    wobbleState = 1;
                }

                if (step == 27){
                    x=-16;
                    wobbleState = 2;
                }

                if (step>=27 && step<47){
                    x+=16;
                    if (x<300){
                        screen_drawAsset(credits2b_mid_asset, x,yPos[x/16] + 120,16,63,x,0); 
                    }
                }

                if (step == 48){
                    wobbleStrength = 3;
                    wobbleState = 1;
                }
            }

        }

        if (wobbleState > 0){
            // update BPLCON1
            copperIndex = extentionIndex;
            for (scanline = 50; scanline < 255; scanline++) {

                val = 0;
                if (wobbleState == 1){
                    val = math_sin16((frameCount + scanline) << wobbleStrength);
                    if (wobbleStrength == 3){
                        val = (val * (255 - scanline)) / 205;
                    }
                }
                
                copperIndex += 3;
                copperList[copperIndex++] = val << 4 | val;

                  if (scanline == 170){
                    // skip colors
                    copperIndex += 32;
                  }
            }
            
        }

        if (wobbleState == 2){
            wobbleState = 0;
        }

        WaitTOF();

       
        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
            
        }
    }
}