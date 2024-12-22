MEMORY {
	RAM:  start = $0000, size = $C000;
	VRAM: start = $C000, size = $0400;
	
	IO:   start = $E000, size = $1000;
	BIOS: start = $F000, size = $1000;
}

SEGMENTS {
	CODE:   load = BIOS, type = ro;
	DATA:   load = BIOS, type = ro;
	VEC:    load = BIOS, start = $fffa, type = overwrite;
	
	BSS:    load = RAM,  start = $0000, type = bss, define = yes;
}