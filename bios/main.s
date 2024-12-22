
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
	
	store8 'L', IO_CON
	
	;make sure we don't get interrupted by an NMI
	store16 h_null, nmi_h
	store16 h_null, irq_h
	
	;init the FDC
	lda #$04
	sta IO_FCMD
@init:
	lda IO_FSTA
	and #%10000000
	bze @init
	
	lda #$02
	sta z_sector
	store16 w_disk_buf, z_addr
	jsr read_sector
	
	jmp wait

@floppy_ok:
	
	
	jmp wait

wait:
	jmp wait

read_sector:
	pushall
	sei
	
	lda #'_'
	sta IO_CON
	
	;seek to location
	lda z_sector
	sta IO_FDAT
	
	lda #$01
	sta IO_FCMD
	
@seek:
	lda IO_FSTA
	and #%00000100
	bne @seek
	
	
	lda #'S'
	sta IO_CON
	
	;read the desired sector
	
	
	lda z_sector
	sta IO_FDAT
	lda #$02
	sta IO_FCMD
@read:
	jmp @read
	
@end:
	cli
	popall
	rts