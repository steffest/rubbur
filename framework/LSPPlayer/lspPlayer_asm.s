; LightSpeedPlayer assembly wrapper for C
; Exports LSP functions with C calling convention

	XDEF	_LSP_MusicDriver_CIA_Start
	XDEF	_LSP_MusicDriver_CIA_Stop

; Include the LSP player code (relative to this file's location)
	include	"framework/LSPPlayer/LightSpeedPlayer_cia.asm"
	include	"framework/LSPPlayer/LightSpeedPlayer.asm"

; C wrapper for LSP_MusicDriver_CIA_Start
; Parameters already in registers from SDI_compiler.h REG() macros:
; a0 = music data
; a1 = sound bank  
; a2 = VBR
; d0 = PAL/NTSC flag
_LSP_MusicDriver_CIA_Start:
	bsr	LSP_MusicDriver_CIA_Start
	rts

; C wrapper for LSP_MusicDriver_CIA_Stop
_LSP_MusicDriver_CIA_Stop:
	bsr	LSP_MusicDriver_CIA_Stop
	rts

