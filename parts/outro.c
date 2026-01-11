#include "app.h"


BYTE outro_asset;
BYTE steffest_asset;

static UWORD outro_palette[] = {0X000,0X300,0X022,0X402,0X034,0X537,0X730,0Xa14,0X744,0X939,0X261,0X067,0Xc40,0X079,0X679,0Xf06,0Xd56,0X09a,0X0ab,0X6a3,0X4ab,0Xf60,0X0b5,0X0bc,0X0c0,0Xf9a,0X0cd,0X9cd,0X0ef,0Xfce,0Xfd7,0Xfff};
static UWORD steffest_palette[] = {0X000,0X210,0X106,0X420,0X424,0X703,0X52b,0X739,0X740,0Xb22,0X74a,0X64f,0X95a,0Xd40,0Xb60,0X777,0Xe46,0Xb74,0Xd61,0Xe60,0Xf70,0Xf90,0Xfa0,0Xfa0,0Xfb3,0Xace,0Xfb0,0Xfc6,0Xdde,0Xfeb,0Xfff,0Xfff};

void outro_preload(){
    loader_next();
    outro_asset = asset_loadImage("data/rubbursmall.planes", 144, 70, 5, MEMF_ANY);
    loader_next();
    steffest_asset = asset_loadImage("data/steffest.planes", 64, 95, 5, MEMF_ANY);
}

void outro(){
    UBYTE loop = 1;
    UWORD frameCount = 0;
    UWORD yOffset = 0;
    UBYTE modStep;
    UBYTE step = 0;
    UWORD extentionIndex = 0;
    UWORD copperIndex = 0;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline;
    UWORD wobbleIndex = 0;
    UBYTE i;
    UBYTE val;
    SHORT blend = 0;
    

    palette_set(outro_palette,32);
    extentionIndex = copper_reset();
    copperIndex = extentionIndex;
    screen_reset();
    screen_clear();

    asset_moveToChip(outro_asset);
    asset_moveToChip(steffest_asset);

    screen_disableDoubleBuffer();
    WaitTOF();

    screen_drawAsset(outro_asset, 84, 20, 144, 70, 0, 0);


    // setup copperlist
    for (scanline = 64; scanline < 134; scanline++) {

        // wait for line
        copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
        copperList[copperIndex++] = 0xfffe;

        copperList[copperIndex++] = BPLCON1;
        copperList[copperIndex++] = 0;
    }

    // set palette
    scanline = 150;
    copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
    copperList[copperIndex++] = 0xfffe;
    for (i = 0; i < 32; i++){
        copperList[copperIndex++] = 0x0180 + (i << 1);
        copperList[copperIndex++] = steffest_palette[i];
    }


    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;
    

    while (loop){
        frameCount++;

        modStep = mod_isStep();
        copperIndex = extentionIndex;

        if (modStep > 0) {
            step++;

            if (step == 2){
                for (scanline = 64; scanline < 134; scanline++) {
                    val = math_sin16((40 + scanline) << 2);

                    copperIndex += 3;
                    copperList[copperIndex++] = val << 4 | val;       
                }

            }

            if (step == 3){
                for (scanline = 64; scanline < 134; scanline++) {
                    val = math_sin16((20 + scanline) << 2);

                    copperIndex += 3;
                    copperList[copperIndex++] = val << 4 | val;       
                }

            }

            if (step == 4){
                for (scanline = 64; scanline < 134; scanline++) {
                    copperIndex += 3;
                    copperList[copperIndex++] = 0;            
                }
            }

            if (step == 5){
                screen_drawAsset(steffest_asset, 96, 110, 32, 10, 0, 0);
            }

            if (step == 6){
                screen_drawAsset(steffest_asset, 96 + 32, 110, 32, 10, 32, 0);
            }

            if (step == 7){
                screen_drawAsset(steffest_asset, 96 + 64, 110, 64, 10, 0, 11);
            }

            if (step == 8){
                screen_drawAsset(steffest_asset, 124, 124, 64, 73, 0, 22);
            }
        }

        if (step>8){

            if (blend<255){
                blend += 5;
                if (blend > 255) blend = 255;
                palette_fade(outro_palette, 32, blend, 0);


                copperIndex = extentionIndex + 282;
                // Fade out copper set palette
                for (i = 0; i < 32; i++){
                    copperIndex++;
                    copperList[copperIndex++] = palette_blendColor(steffest_palette[i], 0, blend);
                }
            }else{
                loop = 0;
            }


        }
        

        WaitTOF();

    
        if (WRITE_DEBUG && isMouseDown()){
            loop = 0;
        }

        if (loop == 0){
            asset_free(outro_asset);
            asset_free(steffest_asset);
        }


        if (loop==0){
             for (i=1;i<20;i++){
                printf("\n");
            }
            printf("------------------------\n");
            printf("\n");
            printf("        RUBBUR\n");
            printf("\n");
            printf("------------------------\n");
            printf("\n");
            printf("      By Steffest\n");
            printf("\n");
            printf(" Released at RSYNC 2026\n");
            printf("  10 January - Leuven\n");
            printf("\n");
            printf("\n");
            printf("  Thanks for watching!\n");
            for (i=1;i<5;i++){
                printf("\n");
            }

        }
    }
}