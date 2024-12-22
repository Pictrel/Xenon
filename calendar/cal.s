.segment "CODE"

.include "../bios/xenon_def.s"

main:
	lda #'H'
	sta IO_CON
	lda #'E'
	sta IO_CON
	lda #'L'
	sta IO_CON
	lda #'L'
	sta IO_CON
	lda #'O'
	sta IO_CON
@loop:
	jmp @loop