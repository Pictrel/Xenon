.list on
.include "xenon_def.s"

.segment "CODE"

reset:
	lda #$00
	sta z_tick
	sta z_oam_size
	
	ldx $00
	
@loop:
	txa
	asl
	asl
	tay
	
	;type & color
	lda #$75
	sta $C000,y
	
	;x position
	tya
	asl
	iny
	sta $C000,y
	
	;y position
	iny
	
	;character
	lda copyright_str,x
	bze @after_loop
	iny
	sta $C000,y
	
	inc z_oam_size
	
	inx
	jmp @loop
	
@after_loop:
	
	;copy the tile data
	ldx #$00
	
@copy_loop:
	lda tileset_bios,x
	sta $D000,x
	inx
	bne @copy_loop
	
	;place tiles
	
	ldx #$00
	ldy #$00
@tilemap_loop:
	lda tilemap_bios,x
	sta VRAM_TILEMAP,y
	lda #$00
	sta VRAM_ATTRMAP,y
	
	inx
	iny
	
	;check if we reached the end
	txa
	cmp #64
	bcs @after_tilemap_loop
	
	;is y 8, 16, 24...
	tya
	and #%00000111
	bze @addoffset
	
	jmp @tilemap_loop
@addoffset:
	tya
	clc
	adc #24
	tay
	jmp @tilemap_loop

@after_tilemap_loop:

@halt:
	jmp @halt

nmi:
	inc IO_VMX
	inc IO_VMY
	
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
