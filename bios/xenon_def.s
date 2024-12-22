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
IO_FCMD = $E083
IO_FDAT = $E084

IO_ICTL = $E0FF

VRAM_START   = $C000
VRAM_OAM     = $C000
VRAM_PAL     = $C400
VRAM_TILEMAP = $C800
VRAM_ATTRMAP = $CC00
VRAM_TILESET = $D000 ;64 4bpp tiles