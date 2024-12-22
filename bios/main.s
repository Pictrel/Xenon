
main_irq:
	sei
	
	ldx z_cyc
	lda pal_cycle,x
	sta $C402
	
	ldx z_cyc
	inx
	txa
	and #%00011111
	sta z_cyc
	
	;lda IO_VYC
	lda IO_VYC
	add #8
	sta IO_VYC

@end:
	
	cli
	rti


main_nmi:
	pushall
	sei
	
	lda IO_JOY1
	sta IO_CON
	
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
	
	lda IO_JOY1
	ora IO_JOY2
	and #BTN_START
	bnz main_loadgame
	
	cli
	popall
	rti

main_loadgame:
	sei
	
	lda #'L'
	sta IO_CON
	
	;make sure we don't get interrupted by an NMI
	store16 h_null, nmi_h
	store16 h_null, irq_h
	
	;enable interrupts
	store16 @floppy_ok, irq_h
	lda #%00000010
	sta IO_ICTL
	cli
	
	;init the FDC
	lda #$04
	sta IO_FCMD
	
	jmp wait

@floppy_ok:
	lda #'O'
	sta IO_CON
	lda #'K'
	sta IO_CON
	lda #$0a
	sta IO_CON
	
	
	
	cli
	rti


wait:
	jmp wait