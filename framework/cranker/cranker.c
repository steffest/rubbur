#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "cranker.h"


/* Decompress cranked data that's already in memory */
UBYTE *crankerDecompress(UBYTE *compressedData, ULONG compressedSize, UBYTE memoryType) {
    UBYTE *decompressedData = NULL;
    ULONG decompressedSize;
    
    /* Validate input */
    if (!compressedData || compressedSize < 4) {
        return NULL;
    }
    
    /* Extract decompressed size from header
     * Header format: $b0 followed by 24-bit big-endian size
     * Byte 0: $b0 (marker)
     * Byte 1: high byte of size
     * Byte 2: middle byte of size
     * Byte 3: low byte of size
     */
    decompressedSize = ((ULONG)compressedData[1] << 16) | 
                       ((ULONG)compressedData[2] << 8) | 
                       ((ULONG)compressedData[3]);
    
    /* Allocate memory for decompressed data */
    decompressedData = (UBYTE *)AllocMem(decompressedSize, memoryType);
    if (!decompressedData) {
        return NULL;
    }
    
    /* Decompress using assembly routine (skip 4-byte header) */
    lzodecrunch(compressedData + 4, decompressedData);
    
    return decompressedData;
}
