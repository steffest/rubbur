#ifndef CRANKER_H
#define CRANKER_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef SDI_COMPILER_H
#include <SDI_compiler.h>
#endif

/* Assembly decompression routine
 * a0 = source (compressed data, skip 4 byte header)
 * a1 = destination
 */
void ASM lzodecrunch(
    REG(a0, void *source),
    REG(a1, void *destination)
);

/* Decompress cranked data that's already in memory
 * compressedData: pointer to compressed data (with 4-byte header)
 * compressedSize: size of compressed data including header
 * memoryType: MEMF_CHIP or MEMF_ANY for the decompressed buffer
 * Returns: pointer to decompressed data, or NULL on error
 */
UBYTE *crankerDecompress(UBYTE *compressedData, ULONG compressedSize, UBYTE memoryType);

#endif
