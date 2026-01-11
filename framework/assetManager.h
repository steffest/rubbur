#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <exec/types.h>

struct ASSET {
    UBYTE type;
    UBYTE memoryType;
    UBYTE planeCount;
    UWORD planeSize;
    UWORD width;
    UWORD height;
    UBYTE *data;
    int size;
};


BYTE asset_loadFile(char *filename, UBYTE memoryType);
BYTE asset_fromMemory(UBYTE *data, ULONG size, UBYTE memoryType);
BYTE asset_loadImage(char *filename, UWORD width, UWORD height, UBYTE bitplanes, UBYTE memoryType);
BYTE asset_loadImageCompressed(char *filename, UWORD width, UWORD height, UBYTE bitplanes, UBYTE memoryType);
BYTE asset_deCompress(BYTE index, UBYTE memoryType);
UBYTE *getAssetData(BYTE index);
int getAssetSize(BYTE index);
BOOL asset_moveToChip(BYTE index);
struct ASSET *getAsset(BYTE index);
void free_assets(void);
BYTE asset_flipImageHorizontal(BYTE index);
void asset_free(BYTE index);

#endif // ASSET_MANAGER_H