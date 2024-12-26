MEMORY {
	RAM:  start = $0000, size = $C000;
	VRAM: start = $C000, size = $2000;
	
	IO:   start = $E000, size = $1000;
	BIOS: start = $F000, size = $1000;
}

SEGMENTS {
	CODE:     load = BIOS, type = ro;
	DATA:     load = BIOS, type = ro;
	API:      load = BIOS, start = $f800, type = overwrite;
	APITABLE: load = BIOS, start = $ff00, type = overwrite;
	VEC:      load = BIOS, start = $fffa, type = overwrite;
	
	ZEROPAGE: load = RAM,  start = $0000, type = bss, define = yes;
	STACK:    load = RAM,  start = $0100, type = bss, define = yes;
	BSS:      load = RAM,  start = $0200, type = bss, define = yes;
}