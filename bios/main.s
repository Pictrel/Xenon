
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
	
	;load sector 0
	lda #$00
	sta z_sector
	store16 w_disk_buf, z_addr
	jsr read_sector
	
	;check if the disk is valid
	ldx #$00
@compare_loop:
	lda w_disk_buf,x
	cmp file_header,x
	bne @not_valid
	inx
	cpx #4
	bne @compare_loop
	
	;the disk is valid. now copy main disk info (entry & boot)
	lda #$0f
	sta VRAM_OAM+3
	
	lda w_disk_buf+$16
	sta xe_boot
	lda w_disk_buf+$1c
	sta xe_entry
	lda w_disk_buf+$1d
	sta xe_entry+1
	
	
	
@find_loop_inx:
	
	jmp wait
	
@not_valid:
	lda #$0e
	sta VRAM_OAM+3
	jmp wait
	
wait:
	jmp wait


file_header: .byte "XEN6"



get_file:
	;load sector 1
	lda #$01
	sta z_sector
	store16 w_disk_buf, z_addr
	jsr read_sector
	
	;loop over every file until we find the bootfile
	ldx #$00
find_loop:
	lda w_disk_buf,x
	cmp #'F'
	bne @find_loop_invalid_file
	
	lda #'F'
	sta IO_CON
	jmp @find_loop
	
@find_loop_invalid_file:
	lda #'I'
	sta IO_CON
	jmp @find_loop_inx



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
	
	lda #5
	sta IO_FCMD
	lda IO_FDAT
	sta IO_CON
	
	;set the interurpt handler
	store16 @int_handle, irq_h
	lda #%00000010
	sta IO_ICTL
	cli
	
	;read the desired sector
	ldy #$00
	
	lda z_sector
	sta IO_FDAT
	lda #$02
	sta IO_FCMD
	
@read:
	lda IO_FSTA
	cmp #$86
	beq @read

@print:
	lda #'O'
	sta IO_CON
	lda #'K'
	sta IO_CON
	lda #$0a
	sta IO_CON
	
	lda #.lobyte(z_addr)
	sta IO_CON
	lda #.hibyte(z_addr)
	sta IO_CON
	
@end:
	cli
	popall
	rts

@int_handle:
	pha
	
	lda IO_FDAT
	sta (z_addr),y
	sta IO_CON
	
	iny
	bze @next_page
	
	pla
	rti

@next_page:
	lda #'N'
	sta IO_CON
	lda #'P'
	sta IO_CON
	tya
	sta IO_CON
	lda #$0a
	sta IO_CON
	inc z_addr+1 ;increment high addr
	
	pla
	rti