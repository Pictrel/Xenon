.segment "API"

;BIOS API, to be called by disk software
;x = function number
api:
	pushall
	
	lda #'A'
	sta IO_CON
	lda #'P'
	sta IO_CON
	lda #'I'
	sta IO_CON
	txa
	sta IO_CON
	
	;address must be multiplied by 2
	txa
	asl
	tax
	stx z_addr
	lda #$FF
	sta z_addr + 1
	
	;load 16-bit value
	ldy #$01
	ldx #$00
	lda (z_addr,x) ;oh, if only there was indirect mode without a register...
	tax
	lda (z_addr),y
	
	;lo byte in X
	;hi byte in A
	stx z_addr
	sta z_addr,y
	
	
	jmp (z_addr)
@loop:
	jmp @loop





null:
	lda #'i'
	sta IO_CON
	lda #'v'
	sta IO_CON
	
	popall
	rts




;source address in z_src
;destination address in z_dest
memcpy:
	lda #'m'
	sta IO_CON
	lda #'c'
	sta IO_CON
	
	ldy #$00
@L1:
	lda #'1'
	sta IO_CON
	
	lda (z_src),y
	sta (z_dest),y
	
	iny
	bze @overflow
	
@L2:
	lda #'2'
	sta IO_CON
	
	lda z_count
	bmi @L3
	
	;NOT negative (<$80)
	dec z_count
	bmi @underflow
	
	jmp @L1

@L3:
	;negative (>$80)
	dec z_count
	
	jmp @L1
	

@end:
	lda #'E'
	sta IO_CON
	popall
	rts

@underflow:
	lda #'U'
	sta IO_CON
	lda z_count+1
	bmi @underflow_2
	
	;NOT negative
	dec z_count+1
	bmi @end
	jmp @L1

@underflow_2:
	dec z_count+1
	jmp @L1

@overflow:
	lda #'O'
	sta IO_CON
	;inc high bytes
	inc z_src
	inc z_dest
	jmp @L2






.segment "APITABLE"

api_table:
	.word null,   null,   null,   null,   memcpy, null,   null,   null,   null,   null,   null,   null,   null,   null,   null,   null
