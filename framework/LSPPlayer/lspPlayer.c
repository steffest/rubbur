#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include "lspPlayer.h"

static UBYTE *lsp_bankData = NULL;
static UBYTE *lsp_musicData = NULL;
static ULONG lsp_bankSize = 0;
static ULONG lsp_musicSize = 0;

/* Load a file into memory */
static UBYTE *loadFile(char *filename, UBYTE memoryType, ULONG *outSize) {
    BPTR file;
    UBYTE *data = NULL;
    ULONG fileSize;
    ULONG bytesRead;
    
    file = Open(filename, MODE_OLDFILE);
    if (!file) {
        printf("LSP: Failed to open file %s\n", filename);
        return NULL;
    }
    
    /* Get file size */
    Seek(file, 0, OFFSET_END);
    fileSize = Seek(file, 0, OFFSET_BEGINNING);
    
    /* Allocate memory */
    data = (UBYTE *)AllocMem(fileSize, memoryType);
    if (!data) {
        printf("LSP: Failed to allocate memory for %s\n", filename);
        Close(file);
        return NULL;
    }
    
    /* Read file */
    bytesRead = Read(file, data, fileSize);
    Close(file);
    
    if (bytesRead != fileSize) {
        printf("LSP: Failed to read file %s\n", filename);
        FreeMem(data, fileSize);
        return NULL;
    }
    
    if (outSize) {
        *outSize = fileSize;
    }
    
    return data;
}

/* Initialize LSP player and load music files */
BOOL lsp_init(char *bankFilename, char *musicFilename) {
    /* Load sound bank into CHIP memory */
    lsp_bankData = loadFile(bankFilename, MEMF_CHIP, &lsp_bankSize);
    if (!lsp_bankData) {
        return FALSE;
    }
    
    /* Load music data into FAST memory (or ANY if no fast available) */
    lsp_musicData = loadFile(musicFilename, MEMF_ANY, &lsp_musicSize);
    if (!lsp_musicData) {
        FreeMem(lsp_bankData, lsp_bankSize);
        lsp_bankData = NULL;
        return FALSE;
    }
    
    printf("LSP: Loaded bank (%ld bytes) and music (%ld bytes)\n", lsp_bankSize, lsp_musicSize);
    return TRUE;
}

/* Start LSP music playback */
void lsp_start(void) {
    if (!lsp_bankData || !lsp_musicData) {
        printf("LSP: Music not loaded, call lsp_init() first\n");
        return;
    }
    
    /* Call assembly LSP player
     * a0 = music data
     * a1 = sound bank
     * a2 = VBR (0 for 68000)
     * d0 = 0 for PAL, 1 for NTSC
     */
    LSP_MusicDriver_CIA_Start(lsp_musicData, lsp_bankData, NULL, 0);
    printf("LSP: Music started\n");
}

/* Stop LSP music playback */
void lsp_stop(void) {
    LSP_MusicDriver_CIA_Stop();
    printf("LSP: Music stopped\n");
}

/* Free LSP music data */
void lsp_free(void) {
    lsp_stop();
    
    if (lsp_bankData) {
        FreeMem(lsp_bankData, lsp_bankSize);
        lsp_bankData = NULL;
    }
    
    if (lsp_musicData) {
        FreeMem(lsp_musicData, lsp_musicSize);
        lsp_musicData = NULL;
    }
}
