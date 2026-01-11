#include <exec/memory.h>
#include <stdio.h>
#include "app.h"
#include "exec/types.h"

#define TASK_PRIORITY 40

BOOL playMusic = TRUE;
BYTE modIndex;
BYTE sampleIndex = -1;
int mod_size;


int main(int argc, char **argv){
    
    if (WRITE_DEBUG) system_printInfo();

        if (!display_open()) {
            puts("Could not open display");
            return 1;
        }

        SetTaskPri(FindTask(NULL), TASK_PRIORITY);
    
    copper_init();
    screen_init(SCREENWIDTH, SCREENHEIGHT, 5, TRUE);
    copper_activate();

    loader_show();
    warpmode(0);

    if (playMusic){
        loader_next();
        modIndex=-1;
        //if (system_isSomewhatHighEnd()) modIndex = asset_loadFile("rsync5.mod", MEMF_CHIP);
        if (modIndex < 0) {
            // running from disk or on low specs
            //modIndex = asset_loadFile("rsync5_LQ.mod", MEMF_CHIP);
            //modIndex = asset_loadFile("rsync5_LQ.mod.cranked", MEMF_FAST);
            //modIndex = asset_deCompress(modIndex, MEMF_CHIP);
            //modIndex = asset_loadFile("beat.mod", MEMF_CHIP);
            
            //modIndex = asset_loadFile("rubbur.mod", MEMF_CHIP);

            modIndex = asset_loadFile("MOD.rubbur.song", MEMF_FAST);
            sampleIndex = asset_loadFile("MOD.rubbur.smps", MEMF_CHIP);
        }
        loader_next();
    }


    intro_preload();
    dude_preload();
    dude2_preload();
    dancer_preload();
    dancer2_preload();
    walker_preload();
    oldman_preload();
    plasma_preload();
    sequencer_preload();
    credits_preload();
    credits2_preload();
    outro_preload();


    if (WRITE_DEBUG){
        printf("Asset preload done\n");
        printf("Free Chip memory: %d\n", AvailMem(MEMF_CHIP));
        printf("Free Fast memory: %d\n", AvailMem(MEMF_FAST));
    }

    if (playMusic) mod_playAsset(modIndex,sampleIndex,0);
    loader_hide();
   
    intro(); 
    dude(); // mod index 3
    dude2(); // mod index 4
    dancer(); // mod index 5
    dancer2(); // mod index 6
    walker();  // mod index 7                       
    oldman(); // mod index 8     
    plasma(); // mod index 12 
    sequencer(); // mod index 15
    credits();  // mod index 17
    credits2(); // mod index 18
    outro(); // mod index 19
    
    if (playMusic){
        mod_stop();
    }

    free_assets();
    screen_close();
    display_close();
    return 0;
}

