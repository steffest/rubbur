#ifndef LSP_PLAYER_H
#define LSP_PLAYER_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* SAS/C register parameter macros */
#ifndef ASM
#define ASM __asm
#endif

#ifndef REG
#define REG(reg, arg) register __##reg arg
#endif

/* Assembly LSP player functions */
void ASM LSP_MusicDriver_CIA_Start(
    REG(a0, void *musicData),
    REG(a1, void *soundBank),
    REG(a2, void *vbr),
    REG(d0, UWORD palNtsc)
);

void ASM LSP_MusicDriver_CIA_Stop(void);

/* C wrapper functions for easy use */
BOOL lsp_init(char *bankFilename, char *musicFilename);
void lsp_start(void);
void lsp_stop(void);
void lsp_free(void);

#endif
