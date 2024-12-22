.list on
.include "xenon_def.s"

.segment "CODE"

reset:
	lda #$00
	sta z_tick
	sta z_cyc
	
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
	pushall
	sei
	
	;inc IO_VMX
	;inc IO_VMY
	lda #$18
	sta IO_VYC
	lda #%10000000
	sta IO_ICTL
	
	lda #$FC
	sta $C402
	
	inc z_tick
	;inc $C402
	
	lda z_tick
	lsr
	lsr
	sta z_cyc
	
	cli
	popall
	rti

irq:
	sei
	
	ldx z_cyc
	lda pal_cycle,x
	sta $C402
	
	ldx z_cyc
	inx
	txa
@mod_loop:
	cmp #32
	blt @mod_end
	sub #32
	jmp @mod_loop
@mod_end:
	sta  z_cyc
	
	;lda IO_VYC
	lda IO_VYC
	add #8
	sta IO_VYC

@end:
	
	cli
	rti

.segment "DATA"

pal_cycle:
	.byte $E0, $E4, $E8, $EC, $F0, $F4, $F8, $FC
	.byte $DC, $BC, $9C, $7C, $5C, $3C, $1C, $1D 
	.byte $1E, $1F, $1B, $17, $13, $0F, $0B, $07 
	.byte $03, $23, $43, $63, $83, $A3, $C3, $E3
	

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
z_cyc: .res 1

.segment "VEC"

.word nmi
.word reset
.word irq
