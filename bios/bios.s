.list on
.include "xenon_def.s"

.segment "CODE"

reset:
	lda #$00
	sta z_tick
	sta z_oam_size
	
	;copy the tile data
	jsr copy_tileset
	
	;place tiles
	jsr copy_tilemap
	
	lda #$C0
	sta IO_VMX
	lda #$E0
	sta IO_VMY
	
@halt:
	jmp @halt

copy_tileset:
	ldx #$00
@loop:
	lda tileset_bios,x
	sta $D000,x
	inx
	bne @loop
	rts

copy_tilemap:
	ldx #$00
	ldy #$00
@loop:
	lda tilemap_bios,x
	sta VRAM_TILEMAP,y
	lda #$00
	sta VRAM_ATTRMAP,y
	
	inx
	iny
	
	;check if we reached the end
	txa
	cmp #64
	bcs @end
	
	;is y 8, 16, 24...
	tya
	and #%00000111
	bze @addoffset
	
	jmp @loop
@addoffset:
	tya
	clc
	adc #24
	tay
	jmp @loop
@end:
	rts

nmi:
	;inc IO_VMX
	;inc IO_VMY
	inc z_tick
	rti

irq:
	rti

.segment "DATA"
 

copyright_str: .byte " (C) Pictrel 2024 v0.03", $00

.include "graphics.s"

.segment "BSS"

z_tick: .res 1
z_oam_size: .res 1

.segment "VEC"

.word nmi
.word reset
.word irq
