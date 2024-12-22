MEMORY {
	RAM:   start = $0000, size = $A000;
	RAM_P: start = $A000, size = $2000;
	
	VRAM: start = $C000, size = $2000;
	
	IO:   start = $E000, size = $1000;
	BIOS: start = $F000, size = $1000;
}

SEGMENTS {
	BSS:    load = RAM,   start = $0000, type = bss, define = yes;
	CODE:   load = RAM_P, start = $A000, type = rw;
}