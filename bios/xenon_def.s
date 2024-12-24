.macpack generic

.macro pushall
	pha
	
	txa
	pha
	
	tya
	pha
	
	php
.endmacro

.macro popall
	plp
	
	pla
	tay
	
	pla
	tax
	
	pla
.endmacro

.macro store16 src, dest
	lda #.lobyte(src)
	sta dest
	lda #.hibyte(src)
	sta dest+1
.endmacro

.macro store8 src, dest
	lda #src
	sta dest
.endmacro

IO_CON  = $E000
IO_TEST = $E001
IO_JOY1 = $E002
IO_JOY2 = $E003
IO_CNT  = $E004
IO_TCNT = $E005
IO_TCTL = $E006
IO_TDIV = $E007

IO_VCTL = $E010
IO_VSTA = $E011

IO_VMX  = $E014
IO_VMY  = $E015
IO_VY   = $E016
IO_VYC  = $E017

IO_VWL  = $E018
IO_VWR  = $E019

IO_FCTL = $E080
IO_FSTA = $E081
IO_FCMD = $E082
IO_FDAT = $E083

IO_ISTA = $E0FE
IO_ICTL = $E0FF

BTN_UP     = %00000001
BTN_DOWN   = %00000010
BTN_LEFT   = %00000100
BTN_RIGHT  = %00001000
BTN_A      = %00010000
BTN_B      = %00100000
BTN_SELECT = %01000000
BTN_START  = %10000000

VRAM_START   = $C000
VRAM_OAM     = $C000
VRAM_PAL     = $C400
VRAM_TILEMAP = $C800
VRAM_ATTRMAP = $CC00
VRAM_TILESET = $D000 ;64 4bpp tiles

BIOS_API = $F800