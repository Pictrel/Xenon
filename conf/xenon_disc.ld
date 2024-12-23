MEMORY {
	# we must do this or ld65 will output a huge file
	RAM_A: start = $0000, size = $2000;
	RAM_B: start = $2000, size = $2000;
	RAM_C: start = $4000, size = $2000;
	RAM_D: start = $6000, size = $2000;
	RAM_E: start = $8000, size = $2000;
	RAM_F: start = $A000, size = $2000;
	VRAM:  start = $C000, size = $2000;
	
	IO:   start = $E000, size = $1000;
	BIOS: start = $F000, size = $1000;
}

SEGMENTS {
	CODE_0:   load = RAM_0, type = rw;
	CODE_1:   load = RAM_0, type = rw;
	CODE_2:   load = RAM_0, type = rw;
	CODE_3:   load = RAM_0, type = rw;
	CODE_4:   load = RAM_0, type = rw;
	CODE_5:   load = RAM_0, type = rw;
	
	ZEROPAGE: load = RAM,  start = $0000, type = bss, define = yes;
	STACK:    load = RAM,  start = $0100, type = bss, define = yes;
	BSS:      load = RAM,  start = $0200, type = bss, define = yes;
}