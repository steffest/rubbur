#include "app.h"
#include "framework/cranker/cranker.h"

#include <stdio.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <proto/graphics.h>
#include <intuition/intuition.h>
#include <hardware/custom.h>

extern struct Custom far custom;

static struct ASSET assets[50];
char assetCount = -1;

/* Bit-reversal lookup table for horizontal flipping */
static const UBYTE bitReverseTable[256] = {
    0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
    0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
    0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
    0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
    0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
    0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
    0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
    0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
    0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
    0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
    0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
    0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
    0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
    0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
    0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
    0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};

BYTE asset_loadFile(char *filename, UBYTE memoryType){
    // Load a file from the filesystem into memory

    int i;
    UBYTE *memory;
    FILE *fileHandle;
    ULONG fileSize;

    int MEMTYPE = MEMF_ANY;
    if (memoryType==MEMF_CHIP) MEMTYPE = MEMF_CHIP;
    if (memoryType==MEMF_FAST) MEMTYPE = MEMF_FAST;
 
    assetCount++;
    assets[assetCount].type = ASSET_TYPE_LOADED;
    assets[assetCount].memoryType = memoryType;

    // if there is .cranked in the filename, load it as a cranked file
    if (strstr(filename,".cranked")) {
        assets[assetCount].type = ASSET_TYPE_COMPRESSED;
    }

    if (WRITE_DEBUG) printf("Asset: loading file %s into slot %d\n",filename,assetCount);
  
    fileHandle = fopen(filename, "rb");
    if (fileHandle == NULL) {
        printf("Asset Failed to open file %s\n",filename);
        return -1;
    }

    // Get the file size
    fseek(fileHandle,0,2);
    fileSize=ftell(fileHandle);
    if (WRITE_DEBUG_DETAIL) printf("file is %d bytes\n",fileSize);
    rewind(fileHandle);
   
    // Allocate memory
    memory = AllocMem(fileSize, MEMTYPE);
    if (memory == NULL) {
        printf("Asset: Failed to allocate memory for %s\n",filename);
        fclose(fileHandle);
        assets[assetCount].type = ASSET_TYPE_NONE;
        return -1;
    }

    // Read the file into memory
    if (fread(memory, sizeof(UBYTE), fileSize, fileHandle) != fileSize) {
        printf("Failed to read file\n");
        FreeMem(memory, fileSize);
        assets[assetCount].type = ASSET_TYPE_NONE;
        fclose(fileHandle);
        return -1;
    }
    fclose(fileHandle);
 
    if (memoryType == MEMF_ANY){
        if (system_isChipMem(memory)){
            assets[assetCount].memoryType = MEMF_CHIP;
            if (WRITE_DEBUG_DETAIL) printf("File landed in CHIP memory\n");
        }else{
            if (WRITE_DEBUG_DETAIL) printf("File landed in FAST memory\n");
        }
    }

    assets[assetCount].data = memory;
    assets[assetCount].size = fileSize;

    return assetCount;

}

BYTE asset_fromMemory(UBYTE *data, ULONG size, UBYTE memoryType) {
    UBYTE *memory;
    int MEMTYPE = MEMF_ANY;
    if (memoryType==MEMF_CHIP) MEMTYPE = MEMF_CHIP;
    if (memoryType==MEMF_FAST) MEMTYPE = MEMF_FAST;

    assetCount++;
    assets[assetCount].type = ASSET_TYPE_LOADED;
    assets[assetCount].memoryType = memoryType;

    memory = AllocMem(size, MEMTYPE);
    if (memory == NULL) {
        printf("Asset: Failed to allocate memory for embedded asset\n");
        assets[assetCount].type = ASSET_TYPE_NONE;
        return -1;
    }

    CopyMem(data, memory, size);

    assets[assetCount].data = memory;
    assets[assetCount].size = size;

    return assetCount;
}

/*
// Shrinkler compressed files, with header
// leave out for now - it's too slow.
BYTE asset_loadCompressed(char *filename, UBYTE memoryType, UBYTE andDecompress, UWORD fileSize){
    if (andDecompress){
        UBYTE *memory;
        
        int MEMTYPE = MEMF_ANY;
        if (memoryType==MEMF_CHIP) MEMTYPE = MEMF_CHIP;
        if (memoryType==MEMF_FAST) MEMTYPE = MEMF_FAST;

        assetCount++;
        assets[assetCount].type = 1;
        assets[assetCount].memoryType = memoryType;

        memory = shrinklerLoad(filename, memoryType, 0, 0, 0);
        assets[assetCount].data = memory;
        //assets[assetCount].size = 411678;
        assets[assetCount].size = fileSize; // TODO: how to get the size of the decompressed file?

        return assetCount;
    }else{
        return asset_loadFile(filename, memoryType);
    }
    
}


BYTE asset_loadImageCompressed(char *filename, UWORD width, UWORD height, UBYTE bitplanes, UBYTE memoryType, UBYTE andDecompress, UWORD fileSize){
    char index = asset_loadCompressed(filename, memoryType,andDecompress,fileSize);
    if (index<0) return -1;
    assets[index].width = width;
    assets[index].height = height;
    assets[index].planeCount = bitplanes;
    assets[index].planeSize = (width >> 3) * height;
    return index;
}

*/

BYTE asset_loadImage(char *filename, UWORD width, UWORD height, UBYTE bitplanes, UBYTE memoryType){
    BYTE index = asset_loadFile(filename, memoryType);
    if (index<0) return -1;
    assets[index].width = width;
    assets[index].height = height;
    assets[index].planeCount = bitplanes;
    assets[index].planeSize = (width >> 3) * height;
    return index;
}

BYTE asset_loadImageCompressed(char *filename, UWORD width, UWORD height, UBYTE bitplanes, UBYTE memoryType){
    BYTE index = asset_loadFile(filename, memoryType);
    if (index<0) return -1;
    assets[index].width = width;
    assets[index].height = height;
    assets[index].planeCount = bitplanes;
    assets[index].planeSize = (width >> 3) * height;
    assets[index].type = ASSET_TYPE_COMPRESSED;
    return index;
}

/* Decompress a cranked asset in-place
 * Replaces the compressed data with decompressed data
 * Returns the same asset index on success, -1 on failure
 */
BYTE asset_deCompress(BYTE index, UBYTE memoryType){
    UBYTE *decompressedData;
    UBYTE *compressedData;
    ULONG compressedSize;
    ULONG decompressedSize;
    int MEMTYPE = MEMF_ANY;

    if (assets[index].type != ASSET_TYPE_COMPRESSED) {
        if (WRITE_DEBUG) printf("Asset: Asset %d is not compressed\n", index);

        // check if we should move it to chip
        if (memoryType == MEMF_CHIP && assets[index].memoryType != MEMF_CHIP) {
            asset_moveToChip(index);
        }

        return index;
    }else{
        if (WRITE_DEBUG) printf("Asset: Decompressing Asset %d\n", index);
    }
    
    if (memoryType==MEMF_CHIP) MEMTYPE = MEMF_CHIP;
    if (memoryType==MEMF_FAST) MEMTYPE = MEMF_FAST;
    
    compressedData = assets[index].data;
    compressedSize = assets[index].size;
    
    decompressedData = crankerDecompress(compressedData, compressedSize, MEMTYPE);
    if (decompressedData == NULL) {
        if (WRITE_DEBUG) printf("Asset: Failed to decompress cranked asset %d\n", index);
        return -1;
    }
    
    decompressedSize = ((ULONG)compressedData[1] << 16) | 
                       ((ULONG)compressedData[2] << 8) | 
                       ((ULONG)compressedData[3]);
    
    FreeMem(compressedData, compressedSize);
    
    assets[index].data = decompressedData;
    assets[index].size = decompressedSize;
    assets[index].memoryType = memoryType;
    assets[index].type = ASSET_TYPE_LOADED;
    
    return index;
}


UBYTE *getAssetData(BYTE index){
    return assets[index].data;
}

int getAssetSize(BYTE index){
    return assets[index].size;
}

BOOL asset_moveToChip(BYTE index){
    UBYTE *chipMemory;
    if (assets[index].memoryType==MEMF_CHIP) return TRUE;
    chipMemory = AllocMem(assets[index].size, MEMF_CHIP);
    if (chipMemory == NULL) {
        if (WRITE_DEBUG){
            printf("Failed to allocate chip memory while moving asset %d\n",index);
            printf("Free Chip memory: %d , needed: %d\n",AvailMem(MEMF_CHIP),assets[index].size);
        }
        return FALSE;
    }
    CopyMem(assets[index].data, chipMemory, assets[index].size);
    FreeMem(assets[index].data, assets[index].size);
    assets[index].data = chipMemory;
    assets[index].memoryType = MEMF_CHIP;
    return TRUE;
}

struct ASSET *getAsset(BYTE index){
    return &assets[index];
}

void asset_free(BYTE index){
    if (assets[index].type>ASSET_TYPE_NONE){
        int i;
        if (WRITE_DEBUG_DETAIL) printf("freeing asset %d of type %d\n",index,assets[index].type);

        FreeMem(assets[index].data, assets[index].size);
        assets[index].type = ASSET_TYPE_NONE;
    }
}

void free_assets(void){
    int i;
    if (WRITE_DEBUG_DETAIL) printf("Asset: freeing %d\n",assetCount+1);
    for (i=0; i<=assetCount; i++){
        asset_free(i);
    }
}


/* 
Flip image horizontally using the Blitter 
This creates a new asset and returns its index
*/
BYTE asset_flipImageHorizontal(BYTE index){
    struct ASSET *srcAsset;
    struct ASSET *dstAsset;
    UBYTE *srcData;
    UBYTE *dstData;
    UWORD widthBytes;
    UWORD y, x;
    UBYTE plane;
    BYTE newIndex;
    ULONG totalSize;
    
    srcAsset = &assets[index];
    widthBytes = srcAsset->width >> 3;
    totalSize = srcAsset->planeSize * srcAsset->planeCount;
    
    /* Create a copy: allocate new asset */
    assetCount++;
    newIndex = assetCount;
    dstAsset = &assets[newIndex];
    
    /* Copy asset metadata */
    dstAsset->type = srcAsset->type;
    dstAsset->memoryType = srcAsset->memoryType;
    dstAsset->planeCount = srcAsset->planeCount;
    dstAsset->planeSize = srcAsset->planeSize;
    dstAsset->width = srcAsset->width;
    dstAsset->height = srcAsset->height;
    dstAsset->size = srcAsset->size;
    
    /* Allocate memory for flipped copy */
    dstData = AllocMem(totalSize, srcAsset->memoryType == MEMF_CHIP ? MEMF_CHIP : MEMF_ANY);
    if (dstData == NULL) {
        if (WRITE_DEBUG) printf("asset_flipHorizontal: Failed to allocate memory for copy\n");
        assetCount--;
        return index;
    }
    dstAsset->data = dstData;
    srcData = srcAsset->data;
    
    /* Process each bitplane */
    for (plane = 0; plane < srcAsset->planeCount; plane++) {
        UBYTE *srcPlane = srcData + (plane * srcAsset->planeSize);
        UBYTE *dstPlane = dstData + (plane * srcAsset->planeSize);
        
        /* Process each scanline */
        for (y = 0; y < srcAsset->height; y++) {
            UBYTE *srcLine = srcPlane + (y * widthBytes);
            UBYTE *dstLine = dstPlane + (y * widthBytes);
            
            /* Reverse bytes and bits in scanline */
            for (x = 0; x < widthBytes; x++) {
                /* Reverse bit order within each byte and reverse byte order */
                dstLine[widthBytes - 1 - x] = bitReverseTable[srcLine[x]];
            }
        }
    }
    
    return newIndex;
}