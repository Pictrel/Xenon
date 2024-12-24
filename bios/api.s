org $F800

;BIOS API, to be called by disk software
;a = function number
api:
	pushall
	
	sta z_addr
	lda #$FF
	sta z_addr + 1
	
	jmp (z_addr)
	
null:
	rts

memcpy:
	lda #'a'
	sta IO_CON
	
	popall
	rts

org $FF00

api_table:
	.word null,   null,   null,   null,   memcpy, null,   null,   null,   null,   null,   null,   null,   null,   null,   null,   null
