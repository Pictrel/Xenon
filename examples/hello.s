.segment "CODE"

.include "../bios/xenon_def.s"

main:
	jsr copy_tileset
	
@loop:
	jmp @loop


copy_tileset:
	
	rts

.include "graphics.s"

.include "../bios/ram.s"

.BSS

