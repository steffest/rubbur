#include "app.h"
#include "framework/assetManager.h"
#include "framework/effect_screenzoom.h"
#include "stdio.h"
#include "string.h"

extern struct Custom far custom;

BYTE oldman_asset;
BYTE oldman_eyes_asset;
BYTE oldman_mouths_asset;
BYTE oldman_glasses_asset;
BYTE cycle_asset;
extern BYTE dots_asset;
extern BYTE *bar_overlay;

static UWORD oldman_palette[] = {0X000, 0X111, 0X124, 0X322, 0X515, 0X246, 0X555, 0Xb46, 0X966, 0X379, 0Xa61, 0Xb99, 0Xe75, 0Xeb9, 0Xfc2, 0Xfe7,
                                 0X000,0X019,0X712,0X25e,0Xf0c,0Xc79,0Xf4d,0Xf90,0Xf9b,0X3cf,0Xfb7,0Xfef,0Xff0,0Xffd,0Xfff,0Xfff};


static UWORD oldman_bright_palette[] = {0X000,0X019,0X712,0X25e,0Xf0c,0Xc79,0Xf4d,0Xf90,0Xf9b,0X3cf,0Xfb7,0Xfef,0Xff0,0Xffd,0Xfff,0Xfff};

                                 void oldman_preload() {
  loader_next();
  oldman_asset = asset_loadImage("data/oldman.planes", 160, 232, 4, MEMF_ANY);
  loader_next();
  oldman_eyes_asset = asset_loadImage("data/eyes.planes", 64, 352, 4, MEMF_ANY);
  loader_next();
  oldman_mouths_asset = asset_loadImage("data/mouths.planes", 48, 279, 4, MEMF_ANY);
  loader_next();
  oldman_glasses_asset = asset_loadImage("data/oldmanglasses.planes", 64, 32, 5, MEMF_ANY);
  loader_next();
  cycle_asset = asset_loadFile("data/cycle7.chunky", MEMF_ANY); 


  // TODO: only in debug;
  //dots_asset = asset_loadImage("data/dots.planes", 320, 256, 1, MEMF_ANY);

}

void setup_chunky_copperGrid(UWORD extentionIndex){
    UWORD copperIndex = extentionIndex;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline = 0;
    UWORD startLine = 116;
    UWORD endLine = startLine + 32;
    UBYTE i;

    for (scanline = 64; scanline < 255; scanline++) {
      // wait for line  
      copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
      copperList[copperIndex++] = 0xfffe;

      copperList[copperIndex++] = BPLCON1;
      copperList[copperIndex++] = 0;

      // set colors
      if (scanline >= startLine && scanline < endLine){
        if ((scanline - startLine) % 2 == 0){
             for (i = 16; i < 32; i++){
                copperList[copperIndex++] = 0x180 + (i << 1);
                copperList[copperIndex++] = 0;
            }   
        }
      }
    };

    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;
}

void setup_wobble_copper(UWORD extentionIndex){
    UWORD copperIndex = extentionIndex;
    UWORD* copperList = copper_getCopperList();
    USHORT scanline = 0;
    UBYTE i;

    for (scanline = 64; scanline < 256; scanline++) {
      // wait for line  
      copperList[copperIndex++] = (scanline & 0xFF) * 256 + 7;
      copperList[copperIndex++] = 0xfffe;

      copperList[copperIndex++] = BPLCON1;
      copperList[copperIndex++] = 0;
    };

    // end copperlist
    copperList[copperIndex++] = 0xffff;
    copperList[copperIndex++] = 0xfffe;
}

void oldman() {
  UBYTE loop = 1;
  UWORD frameCount = 0;
  UBYTE modStep;
  UWORD mouthIndex = 0;
  UWORD eyeIndex = 0;
  UWORD extentionIndex = 0;
  UWORD copperIndex = 0;
  UWORD* copperList = copper_getCopperList();
  USHORT scanline;
  UWORD frameIndex = 0;
  UBYTE maxFrame = 12;
  UWORD *imagelist;
  UBYTE i;
  SHORT step = 0;
  SHORT strength = 64;
  SHORT strength2 = 0;
  UBYTE strengthStep = 1;
  UBYTE strength2Step = 0;
  UBYTE chunkyActive = 0;
  SHORT blend = 255;
  UBYTE blendStep = 4;
  SHORT bendStrength = 0;
  SHORT bendDirection = 0;
  SHORT topY = 220;
  UBYTE topStep = 3;
  UBYTE measured = 0;
  UBYTE effectSet = 0;
  UBYTE endCommands = 0;
  UBYTE eyesActive = 0;
  UBYTE mouthActive = 0;
  UBYTE wobbleSpeed = 1;
  UBYTE wobbleActive = 0;
  UWORD val;
  UWORD damp;
  UWORD activePart = 1;
  SHORT lensY = 40;
  SHORT lensStep = 1;
  UBYTE hasStep = 0;

  BYTE bendValues[] = {5, -5, 3,-3,1,-10,3,
                      -4,4, -2,3,-1,10,-2,
                      2,-3, 1,-4, 2,-9, 0};

  UBYTE partIndex = 1;

  extentionIndex = copper_reset();
  copperIndex = extentionIndex;

  screen_reset();
  screen_clear();
  palette_setBlack(32);

  //setup_chunky_copperGrid(extentionIndex);
  
  asset_moveToChip(oldman_asset);
  asset_moveToChip(oldman_eyes_asset);
  asset_moveToChip(oldman_mouths_asset);
  asset_moveToChip(oldman_glasses_asset);
  asset_moveToChip(dots_asset);

  screen_disableDoubleBuffer();
  WaitTOF();

  screen_drawAsset(oldman_asset, 70, 10, 160, 232, 0, 0);


  imagelist = getAssetData(cycle_asset);
  maxFrame = (getAssetSize(cycle_asset) / 480)-1;

  //build_effect_screenzoom(0,200,40); // small head
  //build_effect_screenzoom(40,120,20); // big nose

  //build_effect_screenzoom(40,120,100); // double head

  //build_effect_screenzoom(0,60,10); // big forehead

  //build_effect_screenzoom(30,90,12); // big eyes

  set_screenzoom_planeCount(4);
  build_effect_screenzoom(0,210,100); 
  setup_bend(0,170);
  set_bend_strength(bendStrength);
  set_screenzoom_top(topY);


  //palette_set(oldman_palette, 16);


  if (partIndex == 2){
    step = 16;
    blend = 0;
    topY = 0;
    strength = 0;
    endCommands = 1;
    palette_set(oldman_palette, 16);
    set_screenzoom_top(topY);
    activePart = 2;
  }

  if (partIndex == 3){
    step = 43;
    blend = 0;
    topY = 0;
    strength = 0;
    endCommands = 2;
    palette_set(oldman_palette, 16);
    set_screenzoom_top(topY);
    activePart = 3;
  }

  if (partIndex == 4){
    step = 64;
    blend = 0;
    topY = 0;
    strength = 0;
    endCommands = 3;
    palette_set(oldman_palette, 16);
    set_screenzoom_top(topY);
    activePart = 4;
  }

  while (loop) {
    frameCount++;

    modStep = mod_isStep();

    if (modStep == 15){
      endCommands++;
      activePart++;
      if (endCommands == 3){
        printf("Step %d",step);
      }
      if (endCommands>3){
        loop = 0;
      }
      modStep = 0;
    }

    if (modStep > 0) {

      if (hasStep == 0){
        hasStep = 1;
        if (modStep != 1){
          // AAAARGH this is wrong ... we missed a step
          if (WRITE_DEBUG) printf("Missed step at start of Old Man - found %d\n", modStep);
          step++;
        }
      }

      if (modStep<5){
           step++;

           if (activePart == 1){
              if (frameCount>4){ // crap. sometimes, we miss the first beat-sync. I need to fix this.
                  
                  bendDirection = bendDirection * -1;
                  if (measured == 0){
                    // measure how many frames are passed, so we can calculate the bend strength to keep in sync with the beat
                  
                    // bendStrength whould go from -10 to 10
                    if (frameCount > 30){
                      // fast machine
                      bendDirection = 1;
                      bendStrength = -9;
                      topStep = 1;
                      blendStep = 1;
                      strengthStep = 1;
                    }else{
                      // slow machine
                      bendDirection = 3;
                      bendStrength = -9;
                      topStep = 4;
                      blendStep = 4;
                      strengthStep = 4;
                    }
                    measured = 1;
                  }
            }


            // 7
            if (step == 9){
               screen_drawAsset(oldman_mouths_asset, 128, 132, 48, 31, 0, 2 * 31);
            }

            if (step == 13){
                screen_drawAsset(oldman_mouths_asset, 128, 132, 48, 31, 0, 7 * 31);
                build_effect_screenzoom(0,200,60); // small head
                strength2 = 64;
                strength2Step = 1;
            }

            if (step == 14){
                build_effect_screenzoom(0,60,20); // big forehead
                strength2 = 64;
                strength2Step = 1;
            }

              if (step == 15){
                build_effect_screenzoom(30,90,24); // big eyes
                strength2 = 64;
                strength2Step = 1;
            }

              if (step == 16){
                build_effect_screenzoom(40,120,100); // double head
                strength2 = 64;
                strength2Step = 1;
            }
           }

           if (step == 17){
                // start of part 2
                activePart = 2;
                extentionIndex = copper_reset();
                setup_chunky_copperGrid(extentionIndex);
                chunkyActive = 1;
                screen_drawAsset(oldman_glasses_asset, 112, 72, 32, 32, 0, 0);
                screen_drawAsset(oldman_glasses_asset, 160, 72, 32, 32, 32, 0);
                screen_drawAsset(oldman_mouths_asset, 128, 132, 48, 31, 0, 3 * 31);
            }

           if (step == 21){
                eyesActive = 1;
            }

              if (step == 25){
                screen_drawAsset(oldman_mouths_asset, 128, 132, 48, 31, 0, 5 * 31);
                wobbleSpeed++;
            }

            if (step == 30){
                wobbleSpeed++;
            }

           if (step == 44){
                // start of part 3
                activePart = 3;
                reset_effect_screenzoom();

                blend = 0;
                topY = 0;
                strength = 0;
                bendStrength = 0;
                set_screenzoom_top(topY);
                
                bitplane_clear(4);
                screen_reset();
                chunkyActive = 0;
                mouthActive = 1;
                eyesActive = 1;
                set_bend_strength(0);
                build_effect_screenzoom(10, 200, 0);
                setup_lens_profile(100, 20);
                setup_bend(0,0);
                update_lens_position(lensY);
            }

           if (activePart == 4){

            
            if (step == 65){
                // start of part 4
                activePart = 4;
                screen_reset();
                chunkyActive = 0;
                mouthActive = 1;
                eyesActive = 1;
                lensStep = 0;
                lensY = 0;
                wobbleActive = 0;



                generate_bar_overlay();
                // copy dots to overlay
                if (bar_overlay) {
                    WaitBlit();
                    custom.bltcon0 = 0x09F0; // A->D Copy
                    custom.bltcon1 = 0;
                    custom.bltapt = (APTR)getAssetData(dots_asset);
                    custom.bltdpt = bar_overlay;
                    custom.bltamod = 0;
                    custom.bltdmod = 0;
                    custom.bltsize = (256 << 6) | 20; // 256 lines, 20 words (320px)

                    WaitBlit();
                    custom.bltcon0 = 0x09F0;
                    custom.bltcon1 = 0;  
                    custom.bltapt = (APTR)getAssetData(dots_asset);
                    custom.bltdpt = bar_overlay + (256 * 40);
                    custom.bltamod = 0;
                    custom.bltdmod = 0;
                    custom.bltsize = (256 << 6) | 20;
                }

                build_effect_screenzoom(10, 100, 40);
                setup_bend(20,160);

                for (i = 16; i < 32; i++){
                  palette_setColor(i, oldman_palette[i]);
                }

                
                screen_overrideBitPlane(4,bar_overlay);

            }



            if (step == (65+15)){
              disable_screenzoom();
              //extentionIndex = copper_reset();

              // setup BPL1 wobble
              setup_wobble_copper(extentionIndex);
              wobbleActive = 1;
              wobbleSpeed = 3;
           }
          }
      }

      if (activePart == 3 || activePart == 4){
        eyesActive = 0;
        if (modStep > 4){
          eyesActive = 1;
          mouthIndex++;
          if (mouthIndex > 8) mouthIndex = 0;

          if (mouthActive){
            mouthIndex++;
            if (mouthIndex > 8) mouthIndex = 0;
          screen_drawAsset(oldman_mouths_asset, 128, 132, 48, 31, 0, mouthIndex * 31);
        }

          if (activePart == 4){
            if (!wobbleActive){
              set_bend_strength(bendValues[lensStep]);
              lensStep++;
              if (lensStep>20) lensStep = 0;
              effect_screenzoom(0,1);
            }
          }
        }

        
      }

      if (eyesActive){
          eyeIndex++;
          if (eyeIndex > 10) eyeIndex = 0;
          screen_drawAsset(oldman_eyes_asset, 112, 72, 32, 32, 0, eyeIndex * 32);
          screen_drawAsset(oldman_eyes_asset, 160, 72, 32, 32, 32, (10-eyeIndex) * 32);
        }
    }

    // fade in
    if (blend > 0 && step < 12){
       blend -= blendStep;
       if (blend < 0) blend = 0;
       palette_fade(oldman_palette, 16, blend, 0);
    }

    bendStrength += bendDirection;
    if (bendStrength > 20) bendStrength = 20;
    if (bendStrength < -20) bendStrength = -20;
    set_bend_strength(bendStrength>>1);

    effectSet = 0;
    if (activePart == 4) effectSet = 1;

    // move up
    if (topY > 0){
      topY -= topStep;
      if (topY < 0) topY = 0;
      set_screenzoom_top(topY);
      effect_screenzoom(1, 1);
      effectSet = 1;
    }else{
      if (strength>0){
        strength -= strengthStep;
        if (strength < 0) strength = 0;
        effect_screenzoom(strength, 64);
         effectSet = 1;
      }
    }

    if (activePart == 3){
      lensY += lensStep;
      if (lensY > 160) lensStep = -1;
      if (lensY < 41) lensStep = 1;
      update_lens_position(lensY);
    }

    if (activePart == 4){
      lensY++;
      if (lensY > 255) lensY  = 0;
       screen_overrideBitPlane(4,  bar_overlay + (lensY * 40));

       if (wobbleActive){
        copperIndex = extentionIndex;
        for (scanline = 64; scanline < 256; scanline++) {
          // wait for line  
          copperIndex+=2;

          // BPLCON1
          copperIndex++; 
          val = (math_sin16((frameCount + scanline)<<wobbleSpeed));
          
          damp = 255 - scanline;
          val = ((ULONG)val * damp) >> 8;
          
          copperList[copperIndex++] = (val << 4) | val;
        }
       }
        
       
    }

  
    if (chunkyActive){
      frameIndex++;
      if (frameIndex > maxFrame) frameIndex = 0;

      // update copper Grid
      copperIndex = extentionIndex;
       
      for (scanline = 64; scanline < 255; scanline++) {
        // wait for line  
        copperIndex+=2;

        // BPLCON1
        copperIndex++; // Skip Register
        val = (math_sin16((frameCount + scanline)<<wobbleSpeed));
        
        damp = 255 - scanline;
        val = (val * damp) >> 8;
        
        copperList[copperIndex++] = (val << 4) | val;

        // set colors
        if (scanline >= 116 && scanline < 148){
             // Only on even lines relative to start
             if (((scanline - 116) & 1) == 0){
                for (i = 0; i < 16; i++){
                  copperIndex++;
                  copperList[copperIndex++] = imagelist[frameIndex * 240 + (((scanline - 116) >> 1) * 15) + i];
                }   
             }
        }       
      };

    } else{
      
      if (effectSet == 0){
        if (mouthActive){
          effect_screenzoom(1, 1);

        } else{
          strength2 -= strength2Step;
          if (strength2 < 0) strength2 = 0;
          effect_screenzoom(strength2, 64);
        }

      }

    }

    WaitTOF();

    if (WRITE_DEBUG && isMouseDown()) {
      loop = 0;
    }

    if (loop == 0) {
      asset_free(oldman_asset);
      asset_free(oldman_eyes_asset);
      asset_free(oldman_mouths_asset);
      asset_free(oldman_glasses_asset);
      asset_free(cycle_asset);
      if(bar_overlay) FreeMem(bar_overlay, 320/8 * 512);
      palette_setBlack(32);
    }
  }
}