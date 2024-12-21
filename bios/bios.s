.list on
.include "xenon_def.s"

.segment "CODE"

reset:
	lda #$00
	sta z_tick
	sta z_scn
	
	;copy the tile data
	jsr copy_tileset
	
	;place tiles
	jsr copy_tilemap
	
	jsr copy_title
	
	jsr copy_pal
	
	lda #$C0
	sta IO_VMX
	lda #$E0
	sta IO_VMY
	
	lda #%01000000
	sta IO_ICTL
	cli
	
@halt:
	jmp @halt



copy_title:
	ldx #$00
	
@loop:
	lda xenonstr_oam,x
	sta VRAM_OAM,x
	
	inx
	txa
	cmp #(4 * 5)
	bne @loop
	
	rts



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

copy_pal:
	ldx #$00
@loop:
	lda default_pal,x
	sta $C400,x
	
	inx
	txa
	cmp #$20
	bne @loop
	
	rts

nmi:
	;inc IO_VMX
	;inc IO_VMY
	lda #$60
	sta IO_VYC
	lda #%10000000
	sta IO_ICTL
	
	inc z_tick
	inc $C402
	rti

irq:
	inc $C402
	rti

.segment "DATA"

pal_cycle:
	

xenonstr_oam:
	.byte $01, $3C, $70, $0A
	.byte $01, $4C, $70, $0B
	.byte $01, $5c, $70, $0C
	.byte $01, $6c, $70, $0D
	.byte $01, $7c, $70, $0C

default_pal:
	.byte $00, $e4, $ff, $ff
	.byte $00, $00, $00, $00
	.byte $00, $00, $00, $00
	.byte $00, $00, $00, $00
	
	.byte $00, $e4, $e8, $ec
	.byte $00, $00, $00, $00
	.byte $00, $00, $00, $00
	.byte $00, $00, $00, $00

.include "graphics.s"

.segment "BSS"

z_tick: .res 1
z_scn: .res 1

.segment "VEC"

.word nmi
.word reset
.word irq
