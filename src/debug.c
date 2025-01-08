#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <6502.h>
#include <math.h>
#include "config.h"
#include <stdbool.h>
#include "floppy.h"
#include "io.h"
#include <stdbool.h>
#include <raylib.h>
#include <stdint.h>

bool gfx_debugger_enabled = true;

#define MAX_INST_WIDTH 3
#define DISASM_SIZE  512
#define MAX_TRAPS 16
#define MAX_SYMBOLS 256

int MeasureChar(uint8_t c, int scale);
int DrawChar(uint8_t c, int x, int y, int scale, Color col);
int DrawCharMonospace(uint8_t c, int x, int y, int scale, Color col);
int MeasureString(char *s, int scale, int spacing);
int DrawString(uint8_t *s, int x, int y, int scale, int spacing, Color col);
int DrawStringMonospace(uint8_t *s, int x, int y, int scale, int spacing, Color col);

typedef enum {
	NONE = 0,
	ACCU,
	ABSL,
	ABSX,
	ABSY,
	IMMD,
	IMPL,
	INDI,
	XIND,
	INDY,
	RELA,
	ZPAG,
	ZPGX,
	ZPGY
} Imode;

const Color debug_pal[4] = {
	(Color){0x7c, 0x3f, 0x58, 0xff},
	(Color){0xeb, 0x6b, 0x6f, 0xff},
	(Color){0xf9, 0xa8, 0x75, 0xff},
	(Color){0xff, 0xf6, 0xd3, 0xff}
};

const struct {
	char *o;
	Imode m;
} opcodes[256] = {
	{"BRK",IMPL}, {"ORA",XIND}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"ORA",ZPAG}, {"ASL",ZPAG}, {"???",NONE}, 
	{"PHP",IMPL}, {"ORA",IMMD}, {"ASL",ACCU}, {"???",NONE}, {"???",NONE}, {"ORA",ABSL}, {"ASL",ABSL}, {"???",NONE}, 
	{"BPL",RELA}, {"ORA",INDY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"ORA",ZPGX}, {"ASL",ZPGX}, {"???",NONE}, 
	{"CLC",IMPL}, {"ORA",ABSY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"ORA",ABSX}, {"ASL",ABSX}, {"???",NONE}, 
	{"JSR",ABSL}, {"AND",XIND}, {"???",NONE}, {"???",NONE}, {"BIT",ZPAG}, {"AND",ZPAG}, {"ROL",ZPAG}, {"???",NONE}, 
	{"PLP",IMPL}, {"AND",IMMD}, {"ROL",ACCU}, {"???",NONE}, {"BIT",ABSL}, {"AND",ABSL}, {"ROL",ABSL}, {"???",NONE}, 
	{"BMI",RELA}, {"AND",INDY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"AND",ZPGX}, {"ROL",ZPGX}, {"???",NONE}, 
	{"SEC",IMPL}, {"AND",ABSY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"AND",ABSX}, {"ROL",ABSX}, {"???",NONE}, 
	{"RTI",IMPL}, {"EOR",XIND}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"EOR",ZPAG}, {"LSR",ZPAG}, {"???",NONE}, 
	{"PHA",IMPL}, {"EOR",IMMD}, {"LSR",ACCU}, {"???",NONE}, {"JMP",ABSL}, {"EOR",ABSL}, {"LSR",ABSL}, {"???",NONE}, 
	{"BVC",RELA}, {"EOR",INDY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"EOR",ZPGX}, {"LSR",ZPGX}, {"???",NONE}, 
	{"CLI",IMPL}, {"EOR",ABSY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"EOR",ABSX}, {"LSR",ABSX}, {"???",NONE}, 
	{"RTS",IMPL}, {"ADC",XIND}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"ADC",ZPAG}, {"ROR",ZPAG}, {"???",NONE}, 
	{"PLA",IMPL}, {"ADC",IMMD}, {"ROR",ACCU}, {"???",NONE}, {"JMP",INDI}, {"ADC",ABSL}, {"ROR",ABSL}, {"???",NONE}, 
	{"BVS",RELA}, {"ADC",INDY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"ADC",ZPGX}, {"ROR",ZPGX}, {"???",NONE}, 
	{"SEI",IMPL}, {"ADC",ABSY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"ADC",ABSX}, {"ROR",ABSX}, {"???",NONE}, 
	{"???",NONE}, {"STA",XIND}, {"???",NONE}, {"???",NONE}, {"STY",ZPAG}, {"STA",ZPAG}, {"STX",ZPAG}, {"???",NONE}, 
	{"DEY",IMPL}, {"???",NONE}, {"TXA",IMPL}, {"???",NONE}, {"STY",ABSL}, {"STA",ABSL}, {"STX",ABSL}, {"???",NONE}, 
	{"BCC",RELA}, {"STA",INDY}, {"???",NONE}, {"???",NONE}, {"STY",ZPGX}, {"STA",ZPGX}, {"STX",ZPGY}, {"???",NONE}, 
	{"TYA",IMPL}, {"STA",ABSY}, {"TXS",IMPL}, {"???",NONE}, {"???",NONE}, {"STA",ABSX}, {"???",NONE}, {"???",NONE}, 
	{"LDY",IMMD}, {"LDA",XIND}, {"LDX",IMMD}, {"???",NONE}, {"LDY",ZPAG}, {"LDA",ZPAG}, {"LDX",ZPAG}, {"???",NONE}, 
	{"TAY",IMPL}, {"LDA",IMMD}, {"TAX",IMPL}, {"???",NONE}, {"LDY",ABSL}, {"LDA",ABSL}, {"LDX",ABSL}, {"???",NONE}, 
	{"BCS",RELA}, {"LDA",INDY}, {"???",NONE}, {"???",NONE}, {"LDY",ZPGX}, {"LDA",ZPGX}, {"LDX",ZPGY}, {"???",NONE}, 
	{"CLV",IMPL}, {"LDA",ABSY}, {"TSX",IMPL}, {"???",NONE}, {"LDY",ABSX}, {"LDA",ABSX}, {"LDX",ABSY}, {"???",NONE}, 
	{"CPY",IMMD}, {"CMP",XIND}, {"???",NONE}, {"???",NONE}, {"CPY",ZPAG}, {"CMP",ZPAG}, {"DEC",ZPAG}, {"???",NONE}, 
	{"INY",IMPL}, {"CMP",IMMD}, {"DEX",IMPL}, {"???",NONE}, {"CPY",ABSL}, {"CMP",ABSL}, {"DEC",ABSL}, {"???",NONE}, 
	{"BNE",RELA}, {"CMP",INDY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"CMP",ZPGX}, {"DEC",ZPGX}, {"???",NONE}, 
	{"CLD",IMPL}, {"CMP",ABSY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"CMP",ABSX}, {"DEC",ABSX}, {"???",NONE}, 
	{"CPX",IMMD}, {"SBC",XIND}, {"???",NONE}, {"???",NONE}, {"CPX",ZPAG}, {"SBC",ZPAG}, {"INC",ZPAG}, {"???",NONE}, 
	{"INX",IMPL}, {"SBC",IMMD}, {"NOP",IMPL}, {"???",NONE}, {"CPX",ABSL}, {"SBC",ABSL}, {"INC",ABSL}, {"???",NONE}, 
	{"BEQ",RELA}, {"SBC",INDY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"SBC",ZPGX}, {"INC",ZPGX}, {"???",NONE}, 
	{"SED",IMPL}, {"SBC",ABSY}, {"???",NONE}, {"???",NONE}, {"???",NONE}, {"SBC",ABSX}, {"INC",ABSX}, {"???",NONE}
};

struct {
	const char *s;
	int w;
	int trap;
	uint16_t pc;
} disasm_cache[DISASM_SIZE];

struct {
	int addr;
} traps[MAX_TRAPS] = {0};

typedef struct {
	char *name;
	int parent;
	uint16_t address;
} Symbol;

Symbol symbols[MAX_SYMBOLS] = {0};

extern Image fb_b;
extern Image fb_o;
extern Texture gpu_fb_b;
extern Texture gpu_fb_o;

extern M6502 cpu;
extern int cpu_cyc;
bool cpu_running = false;
int cpu_speed = 166420;

enum {
	H_CONS = 0,
	H_MEM = 1,
	H_ASM = 2,
	H_TILESET = 3,
	H_TILEMAP = 4
} hi_sel = H_CONS;
uint16_t mem_sel = 0xF000;
uint16_t mem_pointer = 0xF000;
bool mem_goto = false;
int mem_digit_sel = 0;
uint16_t mem_val;

uint16_t prg_base;
int prg_sel = 0;
int prg_off = 0;
bool prg_jumping = false;
bool unbound = false;

extern bool irq;
extern bool nmi;

Color rgb332(uint8_t n);

bool handle_scn(void);
void render_pixel(int x, int y);
void render_scanline(int y);


int ctoi(char digit) {
	if (digit >= '0' && digit <= '9') return digit - '0';
	if (digit >= 'a' && digit <= 'f') return digit - 'a' + 10;
	
	return -1;
}

void handle_mem_digit(char digit) {
	if (mem_digit_sel) {
		mem_val <<= 4;
		mem_val |= ctoi(digit);
		mem_digit_sel++;
		
		switch (hi_sel) {
			case H_MEM:
				if (prg_jumping) {
					if (mem_digit_sel == 4) {
						mem_sel     = mem_val;
						mem_pointer = mem_val;
						
						mem_digit_sel = 0;
						prg_jumping = false;
					}
				} else {
					if (mem_digit_sel == 2) {
						cpu_write(NULL, mem_sel, mem_val & 0xFF);
						mem_digit_sel = 0;
					};
				}
				break;
			case H_ASM:
				if (mem_digit_sel == 4 && prg_jumping) {
					disassemble(mem_val);
					prg_off = 0;
					prg_sel = 0;
					mem_digit_sel = 0;
					prg_jumping = false;
				};
				break;
		}
		
	} else {
		mem_val = ctoi(digit);
		mem_digit_sel++;
	}
	
	
}

int free_symbol() {
	for (int i=0; i<MAX_SYMBOLS; i++) {
		if (!symbols[i].name) return i;
	}
}

int add_symbol(Symbol symbol) {
	int i = free_symbol();
	
	symbols[i] = symbol;
	
	return i;
}

int get_addr_symbol(uint16_t address) {
	for (int i=0; i<MAX_SYMBOLS; i++) {
		if (symbols[i].address == address) {
			return i;
		}
	}
	return -1;
}



int add_isnt_trap(uint16_t addr) {
	for (int i=0; i<MAX_TRAPS; i++) {
		if (traps[i].addr == 0) {
			traps[i].addr = addr;
			return i;
		}
	}
	
	return -1;
}

int get_inst_trap(uint16_t addr) {
	for (int i=0; i<MAX_TRAPS; i++) {
		if (traps[i].addr == addr) {
			return i;
		}
	}
	
	return -1;
}

void rm_inst_trap(uint16_t addr) {
	for (int i=0; i<MAX_TRAPS; i++) {
		if (traps[i].addr == addr) {
			traps[i].addr = 0;
			return;
		}
	}
}

int get_region(uint16_t addr) {
	switch (addr >> 12) {
		case 0x0: 
		case 0x1: 
		case 0x2: 
		case 0x3: 
		case 0x4: 
		case 0x5: 
		case 0x6: 
		case 0x7: 
		case 0x8: 
		case 0x9: 
		case 0xA: 
		case 0xB: 
			return 0; // wram
		case 0xC: 
		case 0xD: 
			return 1; // vram
		case 0xE: 
			return 2; // i/o
		case 0xF: 
			return 3; // bios
	}
}

//Finds the symbol with the greatest address lesser than ADDR, and same memory area
int get_nearest_symbol(uint16_t addr) {
	int greatest = -1;
	
	for (int i=0; i<MAX_SYMBOLS; i++) {
		if ((symbols[i].address > symbols[greatest].address) &&
		    (symbols[i].address <= addr) &&
		    (get_region(symbols[i].address) ==
		     get_region(addr))) {
		    
		    greatest = i;
		}
	}
	
	return greatest;
}

void sys_step() {
	irq = false;
	nmi = false;
	
	if (IO_VY == SCREEN_H && cpu_cyc % (265 * 4) == 0) {
		nmi = true;
		m6502_nmi(&cpu);
	}
	
	if (cpu_cyc % 66 == 0) irq = fdd_cycle();
	m6502_run(&cpu, 1);
	m6502_irq(&cpu, FALSE);
	cpu_cyc++;
	
	if (cpu_cyc % (265 * 4) == 0) {
		irq = handle_scn();
		render_scanline(IO_VY);
		IO_VY++;
		if (IO_VY > 156) IO_VY = 0;
	}
	
	if (get_inst_trap(cpu.state.pc) > -1) {
		printf("CPU TRAP @ $%04x\n", cpu.state.pc);
		cpu_running = false;
	}
	
}

int get_inst_width(uint16_t addr) {
	switch (opcodes[cpu_read(NULL, addr)].m) {
		case NONE: return 1;
		case ACCU: return 1;
		case ABSL: return 3;
		case ABSX: return 3;
		case ABSY: return 3;
		case IMMD: return 2;
		case IMPL: return 1;
		case INDI: return 3;
		case XIND: return 2;
		case INDY: return 2;
		case RELA: return 2;
		case ZPAG: return 2;
		case ZPGX: return 2;
		case ZPGY: return 2;
	}
}

char *get_label_name(int i) {
	if (symbols[i].parent > -1) {
		return TextFormat("%s.%s:", symbols[symbols[i].parent].name, symbols[i].name);
	} else {
		return TextFormat("%s:", symbols[i].name);
	}
	
}

char *get_sym_name(int i, int offset) {
	if (i == -1) return "NULL";
	
	if (offset) {
		if (symbols[i].parent > -1) {
			return TextFormat("%s.%s+%d", symbols[symbols[i].parent].name, symbols[i].name, offset);
		} else {
			return TextFormat("%s+%d", symbols[i].name, offset);
		}
	} else {
		if (symbols[i].parent > -1) {
			return TextFormat("%s.%s", symbols[symbols[i].parent].name, symbols[i].name);
		} else {
			return TextFormat("%s", symbols[i].name);
		}
	}
	
}

char *get_addr_name(uint16_t addr) {
	
	addr = (cpu_read(NULL, addr+1) << 8) + cpu_read(NULL, addr);
	
	int sym = get_nearest_symbol(addr);
	
	printf("Symbol for addr %04x, %d\n", addr, sym);
	
	if (sym > -1) {
		return get_sym_name(sym, addr - symbols[sym].address);
	} else {
		return TextFormat("$%04X", addr);
	}
}

char *get_inst_name(uint16_t addr) {
	switch (opcodes[cpu_read(NULL, addr)].m) {
		case NONE: return TextFormat("      ---");
		case ACCU: return TextFormat("      %3s A",            opcodes[(uint8_t)cpu_read(NULL, addr)].o);
		case ABSL: return TextFormat("      %3s %s",           opcodes[(uint8_t)cpu_read(NULL, addr)].o, get_addr_name(addr+1));
		case ABSX: return TextFormat("      %3s %s,X",         opcodes[(uint8_t)cpu_read(NULL, addr)].o, get_addr_name(addr+1));
		case ABSY: return TextFormat("      %3s %s,Y",         opcodes[(uint8_t)cpu_read(NULL, addr)].o, get_addr_name(addr+1));
		case IMMD: return TextFormat("      %3s #$%02X",       opcodes[(uint8_t)cpu_read(NULL, addr)].o, (uint8_t)cpu_read(NULL, addr+1));
		case IMPL: return TextFormat("      %3s",              opcodes[(uint8_t)cpu_read(NULL, addr)].o);
		case INDI: return TextFormat("      %3s (%s)",         opcodes[(uint8_t)cpu_read(NULL, addr)].o, get_addr_name(addr+1));
		case XIND: return TextFormat("      %3s (%s,X)",       opcodes[(uint8_t)cpu_read(NULL, addr)].o, get_addr_name(addr+1));
		case INDY: return TextFormat("      %3s (%s),Y",       opcodes[(uint8_t)cpu_read(NULL, addr)].o, get_addr_name(addr+1));
		case RELA: return TextFormat("      %3s $%02X",        opcodes[(uint8_t)cpu_read(NULL, addr)].o, (uint8_t)cpu_read(NULL, addr+1));
		case ZPAG: return TextFormat("      %3s $%02X",        opcodes[(uint8_t)cpu_read(NULL, addr)].o, (uint8_t)cpu_read(NULL, addr+1));
		case ZPGX: return TextFormat("      %3s $%02X,X",      opcodes[(uint8_t)cpu_read(NULL, addr)].o, (uint8_t)cpu_read(NULL, addr+1));
		case ZPGY: return TextFormat("      %3s $%02X,Y",      opcodes[(uint8_t)cpu_read(NULL, addr)].o, (uint8_t)cpu_read(NULL, addr+1));
	}
	
	return "???";
}

void disassemble(uint16_t address) {
	prg_base = address;
	
	bool labeled = false;
	
	for (int i=0; i<DISASM_SIZE; i++) {
		int sym;
		if (((sym = get_addr_symbol(address)) > -1) && !labeled) {
			disasm_cache[i].s    = strdup(get_label_name(sym));
			disasm_cache[i].pc   = address;
			disasm_cache[i].w    = 0;
			disasm_cache[i].trap = -1;
			labeled = true;
		} else {
			labeled = false;
			disasm_cache[i].s    = strdup(get_inst_name(address));
			disasm_cache[i].pc   = address;
			disasm_cache[i].w    = get_inst_width(address);
			disasm_cache[i].trap = get_inst_trap(address);
			
			
			address += get_inst_width(address);
		}
	}
}

void debug_update() {	
	/*if (cpu_running) {
		for (int y=0; y<SCREEN_H + VBLANK_SIZE / 4; y++) {
			if (y == SCREEN_H) {
				m6502_nmi(&cpu);
			}
			
			IO_VY = y;
			handle_scn();
			
			render_scanline(IO_VY);
			
			for (int i=0; i<264; i++) {
				if (i % 66 == 0) fdd_cycle();
				cpu_cyc += m6502_run(&cpu, 1);
				m6502_irq(&cpu, FALSE);
			}
		}
	}*/
	
	SetWindowSize(
		GetRenderWidth()  / (SCREEN_W * 3) * (SCREEN_W * 3),
		GetRenderHeight() / (SCREEN_H * 1) * (SCREEN_H * 1)
	);
	
	for (int i=0; i<(cpu_speed) && cpu_running; i++) {
		sys_step();
	}
	
	if (IsKeyPressed(KEY_A) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = true, cpu_speed = 1;
	if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = false;
	if (IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = true, cpu_speed = 166420;
	if (IsKeyPressed(KEY_H) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = true, cpu_speed = 1060;
	
	if ((IsKeyPressed(KEY_F3) || IsKeyPressedRepeat(KEY_S)) && !cpu_running) sys_step();
	
	if (IsKeyPressed(KEY_D)      && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_ASM;
	if (IsKeyPressed(KEY_M)      && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_MEM;
	if (IsKeyPressed(KEY_ESCAPE) && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_CONS;
	if (IsKeyPressed(KEY_T)      && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_TILESET;
	if (IsKeyPressed(KEY_M)      && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_TILEMAP;
	if (IsKeyPressed(KEY_U) && IsKeyDown(KEY_LEFT_CONTROL)) {
		disassemble(prg_base);
	}
	
	if (hi_sel == H_MEM) {
		if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))       mem_sel -= IsKeyDown(KEY_LEFT_CONTROL) ? 0x80 : 0x08;
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))   mem_sel += IsKeyDown(KEY_LEFT_CONTROL) ? 0x80 : 0x08;
		if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))   mem_sel--;
		if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) mem_sel++;
		
		if (IsKeyPressed(KEY_G)) {
			prg_jumping = true;
			mem_digit_sel = 0;
			mem_val = 0;
		}
		
		if (mem_sel < (mem_pointer))        mem_pointer = (mem_sel & 0xFFF8);
		if (mem_sel > (mem_pointer + 0xA0)) mem_pointer = (mem_sel & 0xFFF8) - 0xA0;
	}
	
	if (IsKeyPressed(KEY_KP_MULTIPLY)) {
		reset_cpu();
	}
	
	if (hi_sel == H_ASM) {
		if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))       prg_sel--;
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))   prg_sel++;
		if (IsKeyPressed(KEY_F2)) {
			if (disasm_cache[prg_sel + prg_off].trap < 0) {
				uint16_t addr = disasm_cache[prg_sel + prg_off].pc;
				
				printf("Adding trap to addr $%04x, @ %d\n", addr, prg_sel + prg_off);
				int trap = add_isnt_trap(addr);
				printf("T: %d\n", trap);
				
			
			} else {
				uint16_t addr = disasm_cache[prg_sel + prg_off].pc;
				
				printf("Trap already exists at address $%04x, @%d; removing\n", addr, prg_sel + prg_off);
				rm_inst_trap(addr);
			}
			
			//update disassembly
			disassemble(prg_base);
		}
		
		if (IsKeyPressed(KEY_F4)) {
			cpu.state.pc = disasm_cache[prg_sel + prg_off].pc;
		}
		
		if (IsKeyPressed(KEY_G)) {
			prg_jumping = true;
			mem_digit_sel = 0;
			mem_val = 0;
		}
		
		if (IsKeyPressed(KEY_F6)) {
			disassemble(cpu.state.pc);
			prg_off = 0;
			prg_sel = 0;
		}
		
		if (prg_sel < 0) {
			prg_sel = 0;
			prg_off--;
		}
		
		if (prg_sel > 31) {
			prg_sel = 31;
			prg_off++;
		}
		
		if (prg_off+31 > DISASM_SIZE-1) {
			disassemble(disasm_cache[DISASM_SIZE-1].pc +
			            disasm_cache[DISASM_SIZE-1].w);
			prg_sel = 0;
			prg_off = 0;
		}
		
		if (prg_off < 0) {
			prg_off++;
		}
	}
	
	char c = GetCharPressed();
	if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) handle_mem_digit(c);
}

/*
void DrawChar(char c, int x, int y, uint8_t color, int scale) {
	for (int i=0; i<6; i++) {
		for (int j=0; j<8; j++) {
			if ((antarex[c][i] >> j) & 1) DrawRectangle(x+i*scale, y+j*scale, scale, scale, rgb332(color));
		}
	}
}

void DrawString(char *s, int x, int y, uint8_t color, int scale) {
	for (int i=0; s[i]; i++) {
		DrawChar(s[i], x, y, color, scale);
		x += 7*scale;
	}
}
*/

char *get_region_name(uint16_t addr) {
	switch (addr >> 12) {
		case 0x0: 
		case 0x1: 
		case 0x2: 
		case 0x3: 
		case 0x4: 
		case 0x5: 
		case 0x6: 
		case 0x7: 
		case 0x8: 
		case 0x9: 
		case 0xA: 
		case 0xB: 
			return "WRAM";
		case 0xC: 
		case 0xD: 
			return "VRAM";
		case 0xE: 
			return "I/O";
		case 0xF: 
			return "BIOS";
	}
}

char *get_state_name() {
	if (cpu_running) {
		switch (cpu_speed) {
			case 166420: return "RUNNING";
			case 1060:   return "SLOWED";
			case 1:      return "ANIMATED";
			default:     return "???";
		}
	} else {
		return "STOPPED";
	}
}

uint8_t get_state_color() {
	if (cpu_running) {
		switch (cpu_speed) {
			case 166420: return 0b00011100;
			case 1060:   return 0b11111100;
			case 1:      return 0b11110000;
			default:     return 0b01001001;
		}
	} else {
		return 0b11100000;
	}
}

void draw_disasm() {
	for (int y=0; y<32; y++) {
		
		DrawStringMonospace(TextFormat("%04X", disasm_cache[y+prg_off].pc), 8 + 7, 168 + y * 8 + 8, 1, 0, WHITE);
		
		for (int i=0; i<disasm_cache[y+prg_off].w; i++) {
			DrawString(TextFormat("%02X", cpu_read(NULL, disasm_cache[y+prg_off].pc + i)),
				9 + 7 * 6 + i * 3 * 6, 168 + y * 8 + 8, 1, 0, WHITE);
		}
		
		if (cpu.state.pc == disasm_cache[y+prg_off].pc) {
			DrawStringMonospace(disasm_cache[y+prg_off].s, 9 + 7 * 7 + MAX_INST_WIDTH * 3 * 6, 168 + y * 8 + 8, 1, 0, GREEN);
		} else if (prg_sel == y) {
			DrawStringMonospace(disasm_cache[y+prg_off].s, 9 + 7 * 7 + MAX_INST_WIDTH * 3 * 6, 168 + y * 8 + 8, 1, 0, GOLD);
		} else if (disasm_cache[y+prg_off].trap > -1) {
			DrawStringMonospace(disasm_cache[y+prg_off].s, 9 + 7 * 7 + MAX_INST_WIDTH * 3 * 6, 168 + y * 8 + 8, 1, 0, RED);
		} else {
			DrawStringMonospace(disasm_cache[y+prg_off].s, 9 + 7 * 7 + MAX_INST_WIDTH * 3 * 6, 168 + y * 8 + 8, 1, 0, WHITE);
		}
		
		
		DrawStringMonospace(TextFormat("%3d", y+prg_off), 8 + 7 + 6 * 43, 168 + y * 8 + 8, 1, 0, WHITE);
		
		/*
		for (int i=0; i<get_inst_width(prg_pointer + offset); i++) {
			DrawString(TextFormat("%02X", cpu_read(NULL, prg_pointer + offset + i)),
				9 + 7 * 6 + i * 3 * 6, 168 + y * 8 + 8, 1, 0, WHITE);
		}
		
		DrawStringMonospace(TextFormat("%3s", get_inst_name(prg_pointer + offset)),
			9 + 7 * 6 + MAX_INST_WIDTH * 3 * 6, 168 + y * 8 + 8, 1, 0,
			cpu.state.pc == prg_pointer + offset ? GREEN : WHITE);
		
		offset += get_inst_width(prg_pointer + offset);*/
	}
}

void draw_mem() {
	for (int y=0; y<21; y++) {
		DrawString(TextFormat("%04X", mem_pointer + y * 8), 8 + 292 + 4 + (7 * 1), 168 + 99 + y * 8, 1, 1, WHITE);
		DrawString(TextFormat("%04X", mem_pointer + y * 8), 9 + 292 + 4 + (7 * 1), 168 + 99 + y * 8, 1, 1, WHITE);
		
		for (int x=0; x<8; x++) {
			uint16_t a = mem_pointer + x + y * 8;
			uint8_t n = cpu_read(NULL, a);
			
			
			DrawString(TextFormat("%02X", n),
					8 + 292 + 4 + x * (7 * 3) + (7 * 7),
					168 + 99 + y * 8, 1, 0, WHITE
			);
			
			if (mem_sel == a)
			DrawString(TextFormat("%02X", n),
					8 + 292 + 4 + x * (7 * 3) + (7 * 7) + 1,
					168 + 99 + y * 8, 1, 0, WHITE
			);
			
			DrawChar((n >= 0x20 && n <= 0x7F) ? n : '.',
					8 + 292 + 4 + x * (7 * 1) + (7 * 32),
					168 + 99 + y * 8, 1, WHITE);
		}
	}
	
	int sym = get_nearest_symbol(mem_sel);
	
	if (sym > -1) {
		DrawStringMonospace(TextFormat("%04x", mem_sel),                       8 + 292 + 4 + (6 * 1), 168 + 99 + 21 * 8, 1, 1, WHITE);
		DrawStringMonospace(get_sym_name(sym, mem_sel - symbols[sym].address), 8 + 292 + 4 + (6 * 7), 168 + 99 + 21 * 8, 1, 1, WHITE);
	}
}

/*
void draw_stack() {
	for (int y=0; y<14; y++) {
		uint16_t addr = 0x0100 + (0xfe - y * 2);
		
		uint16_t value = (cpu_read(NULL, addr+1) << 8) |
		                 (cpu_read(NULL, addr));
		
		int sym = get_nearest_symbol(value);
		
		DrawStringMonospace(TextFormat("%04x", addr),                                                        8 + 416 + 4 + 6 * 2, 10 + 8 * (13 - y + 1), 1, 1, WHITE);
		DrawStringMonospace(TextFormat("%04x (%s)", value, get_sym_name(sym, value - symbols[sym].address)), 8 + 416 + 4 + 6 * 8, 10 + 8 * (13 - y + 1), 1, 1, WHITE);
	}
}
*/

void DrawTile(int x, int y, int chr, int scale) {
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			DrawRectangle(x + i * scale,
			              y + j * scale, scale, scale, debug_pal[get_tile_value(chr, i, j) % 4]);
		}
	}
}

void draw_tileset() {
	for (int i=0; i<256; i++) {
		DrawTile(626 + (i % 16) * 2 * 8,
		         10  + (i / 16) * 2 * 8, i, 2);
	}
}

void draw_tilemap() {
	for (int x=0; x<32; x++) {
		for (int y=0; y<32; y++) {
			DrawTile(910 + (x * 1 * 8),
			         10  + (y * 1 * 8), cpu_read(NULL, 0xC800 + x + y * 32), 1);
		}
	}
	
	for (int x=0; x<200; x++) DrawPixel(910 + (IO_VMX + x)   % 256,  10 + (IO_VMY + 0)   % 256,  GOLD);
	for (int y=0; y<150; y++) DrawPixel(910 + (IO_VMX + 0)   % 256,  10 + (IO_VMY + y)   % 256,  GOLD);
	for (int x=0; x<200; x++) DrawPixel(910 + (IO_VMX + x)   % 256,  10 + (IO_VMY + 150) % 256,  GOLD);
	for (int y=0; y<150; y++) DrawPixel(910 + (IO_VMX + 200) % 256,  10 + (IO_VMY + y)   % 256,  GOLD);
	//DrawRectangleLines(910 + IO_VMX, 10 + IO_VMY, 200, 150, GOLD);
}

void draw_obj() {
	for (int x=0; x<4; x++) {
		DrawStringMonospace(TextFormat("%02x X  Y  T  A", x * 8),
		                   629 + (x * 140),
		                   276,
		                   	1, 1, WHITE);
	}
	
	for (int i=0; i<32; i++) {
		/*DrawRectangleLines(626 + (i / 8) * 70,
		                   288 + (i % 8) * 20, (8 * 2) + 2, (8 * 2) + 2, WHITE);*/
		
		DrawTile(626 + (i / 8) * 140,
		         288 + (i % 8) * 20, cpu_read(NULL, 0xC003 + i * 4), 2);
		
		DrawStringMonospace(TextFormat("%02x %02x %02x %02x %c%c%1d",
		                                            cpu_read(NULL, 0xC001 + i * 4),
		                                            cpu_read(NULL, 0xC002 + i * 4),
		                                            cpu_read(NULL, 0xC003 + i * 4),
		                                            cpu_read(NULL, 0xC000 + i * 4),
		                                            cpu_read(NULL, 0xC000 + i * 4) & 0x08 ? 'H' : '-',
		                                            cpu_read(NULL, 0xC000 + i * 4) & 0x04 ? 'V' : '-',
		                                            cpu_read(NULL, 0xC000 + i * 4) & 0x03
		                                            ),
		                   648 + (i / 8) * 140,
		                   292 + (i % 8) * 20,
		                   	1, 1, WHITE);
	}
}

void debug_draw() {
	//SetTargetFPS(2);
	
	//GuiEnable();
	
	ClearBackground(BLACK);
	
	if (IsTextureValid(gpu_fb_b)) UnloadTexture(gpu_fb_b);
		gpu_fb_b = LoadTextureFromImage(fb_b);
	
	if (IsTextureValid(gpu_fb_o)) UnloadTexture(gpu_fb_o);
	gpu_fb_o = LoadTextureFromImage(fb_o);
	
	// Display 
	DrawRectangleLines(8,  8, 200+4, 150+4, WHITE);
	DrawRectangleLines(9,  9, 200+2, 150+2, WHITE);
	DrawTexture(gpu_fb_b, 10, 10, WHITE);
	DrawTexture(gpu_fb_o, 10, 10, WHITE);
	DrawRectangle(10 + ((cpu_cyc) % 265*4) / 4 - 1,
	              10 + IO_VY                   - 1, 3, 3, (Color){0xff, 0x00, 0xff, 0xff});
	
	// Assembly Viewer
	DrawRectangle(8, 168, 292 - 4, 450 - 168 - 8, rgb332((hi_sel == H_ASM) ? 0x02 : 0x01));
	draw_disasm();
	
	// Memory viewer
	DrawRectangle(8 + 292 + 4, 168 + 91, 292 - 4, 182, rgb332((hi_sel == H_MEM) ? 0x02 : 0x01));
	draw_mem();
	
	if (GetRenderWidth() >= SCREEN_W * 6) {
		// Tileset viewer
		DrawRectangleLines(624,  8, 256+4, 256+4, WHITE);
		DrawRectangleLines(625,  9, 256+2, 256+2, WHITE);
		draw_tileset();
		
		// Tilemap viewer
		DrawRectangleLines(908,  8, 256+4, 256+4, WHITE);
		DrawRectangleLines(909,  9, 256+2, 256+2, WHITE);
		draw_tilemap();
		
		
		// OAM viewer
		draw_obj();
	}
	
	// Stack trace
	//DrawRectangle(8 + 416 + 4, 10, 600 - 8 - 416 - 16, 16 * 8, rgb332(0x01));
	//draw_stack();
	
	// Registers
	DrawStringMonospace(TextFormat("A:  %02X\n", cpu.state.a),  220, 10,          2, 0, WHITE);
	DrawStringMonospace(TextFormat("X:  %02X\n", cpu.state.x),  220, 10 + 16 * 1, 2, 0, WHITE);
	DrawStringMonospace(TextFormat("Y:  %02X\n", cpu.state.y),  220, 10 + 16 * 2, 2, 0, WHITE);
	DrawStringMonospace(TextFormat("S:  %02X\n", cpu.state.s),  220, 10 + 16 * 3, 2, 0, WHITE);
	DrawStringMonospace(TextFormat("PC: %04X (%s)\n", cpu.state.pc, get_region_name(cpu.state.pc)), 220, 10 + 16 * 4, 2, 0, WHITE);
	DrawStringMonospace(TextFormat("%c%c%c%c%c%c%c%c\n",
		(cpu.state.p & 0x80) ? 'N' : '-',
		(cpu.state.p & 0x40) ? 'V' : '-',
		(cpu.state.p & 0x20) ? '-' : '-',
		(cpu.state.p & 0x10) ? 'B' : '-',
		(cpu.state.p & 0x08) ? 'D' : '-',
		(cpu.state.p & 0x04) ? 'I' : '-',
		(cpu.state.p & 0x02) ? 'Z' : '-',
		(cpu.state.p & 0x01) ? 'C' : '-'), 220, 10 + 16 * 6, 2, 0, WHITE);
	DrawStringMonospace(TextFormat("NMI"),                  220,           10 + 16 * 8, 2, 0, nmi?GOLD:WHITE);
	DrawStringMonospace(TextFormat("IRQ"),                  220 + 14 * 4,  10 + 16 * 8, 2, 0, irq?GOLD:WHITE);
	DrawStringMonospace(TextFormat("%s", get_state_name()), 220 + 14 * 8,  10 + 16 * 8, 2, 0, WHITE);
	
	if (prg_jumping) {
		DrawStringMonospace(TextFormat("GOTO %04X", mem_val), 220 + 14 * 16, 10 + 16 * 8, 2, 0, WHITE);
	}
	
	// I/O
	DrawStringMonospace(TextFormat("VY:   %02X", IO_VY),       292 + 16 + 14 * 1,  168 + 16 * 0, 2, 1, WHITE);
	DrawStringMonospace(TextFormat("VYC:  %02X", IO_VYC),      292 + 16 + 14 * 11, 168 + 16 * 0, 2, 1, WHITE);
	DrawStringMonospace(TextFormat("VMX:  %02X", IO_VMX),      292 + 16 + 14 * 1,  168 + 16 * 1, 2, 1, WHITE);
	DrawStringMonospace(TextFormat("VMY:  %02X", IO_VMY),      292 + 16 + 14 * 11, 168 + 16 * 1, 2, 1, WHITE);
	DrawStringMonospace(TextFormat("FSTA: %02X", get_FSTA()),  292 + 16 + 14 * 1,  168 + 16 * 2, 2, 1, WHITE);
	DrawStringMonospace(TextFormat("FPOS: %02X", fd_pos),      292 + 16 + 14 * 11, 168 + 16 * 2, 2, 1, WHITE);
	DrawStringMonospace(TextFormat("CYC:  %08X", cpu_cyc),     292 + 16 + 14 * 1,  168 + 16 * 3, 2, 1, WHITE);
	
	
	
	DrawFPS(0, 0);
}

void parse_lab_file(char *filename) {
	FILE *fp;
	
	fp = fopen(filename, "r");
	if (!fp) {
		printf("Unable to open bios.lab. Ignoring.\n");
		return;
	}
	
	char buffer[256];
	
	fread(buffer, 1, 9, fp);
	
	if (strncmp(buffer, "XENLAB06\n", 9)) {
		printf("Error: invalid list file\n");
		return;
	}
	
	int latest_parent = -1;
	int i=0;
	while (fgets(buffer, 255, fp)) {
		Symbol sym;
		
		buffer[strlen(buffer)-1] = 0;
		printf("'%-64s' ", buffer);
		
		char *p = buffer;
		
		// address
		sym.address = strtol(p, &p, 0);
		p++;
		
		// name
		sym.name = strdup(p);
		
		if (sym.name[0] == '.') {
			sym.parent    = latest_parent;
			sym.name++;
		} else {
			latest_parent = free_symbol();
			sym.parent    = -1;
		}
		
		int index = add_symbol(sym);
		
		switch (get_region(sym.address)) {
			case 0: printf("\033[34mW "); break;
			case 1: printf("\033[32mV "); break;
			case 2: printf("\033[33mI "); break;
			case 3: printf("\033[31mB "); break;
		}
		
		printf(" [%3d] %04x, %s%s \033[0m %d\n", index, sym.address, (sym.parent > -1) ? TextFormat("%s.", symbols[sym.parent].name) : "", sym.name, sym.address >> 12);
	}
}


void debug_init() {
	parse_lab_file("bios.lab");
	parse_lab_file("xenon.lab");
	
	disassemble(0xF000);
	
	SetExitKey(0);

	SetWindowMinSize(SCREEN_W * 3,  SCREEN_H * 3);
	SetWindowMaxSize(SCREEN_W * 6,  SCREEN_H * 3);
	SetWindowSize(SCREEN_W * 5,  SCREEN_H * 3);
}

/*
void draw(void) {
		ClearBackground(BLACK);
		
		if (IsTextureValid(gpu_fb_b)) UnloadTexture(gpu_fb_b);
		gpu_fb_b = LoadTextureFromImage(fb_b);
		
		if (IsTextureValid(gpu_fb_o)) UnloadTexture(gpu_fb_o);
		gpu_fb_o = LoadTextureFromImage(fb_o);
		
		const int SCALE = fmin(GetRenderWidth() / SCREEN_W,
		                       GetRenderHeight() / SCREEN_H);
		
		const int w = 200 * SCALE;
		const int h = 150 * SCALE;
		
		if (USE_LETTERBOX) {
			DrawTexturePro(gpu_fb_b,
				(Rectangle){0, 0, SCREEN_W, SCREEN_H},
				(Rectangle){GetRenderWidth()  / 2 - w / 2, 
				            GetRenderHeight() / 2 - h / 2, w, h},
				(Vector2){0, 0}, 0.0f, WHITE);
			DrawTexturePro(gpu_fb_o,
				(Rectangle){0, 0, SCREEN_W, SCREEN_H},
				(Rectangle){GetRenderWidth()  / 2 - w / 2, 
				            GetRenderHeight() / 2 - h / 2, w, h},
				(Vector2){0, 0}, 0.0f, WHITE);
		} else {
			DrawTexturePro(gpu_fb_b, 
				(Rectangle){0, 0, SCREEN_W, SCREEN_H},
				(Rectangle){0, 0, GetRenderWidth(), GetRenderHeight()},
				(Vector2){0, 0}, 0.0f, WHITE);
			DrawTexturePro(gpu_fb_o, 
				(Rectangle){0, 0, SCREEN_W, SCREEN_H},
				(Rectangle){0, 0, GetRenderWidth(), GetRenderHeight()},
				(Vector2){0, 0}, 0.0f, WHITE);
		}
		
		DrawFPS(0, 0); 
	} 
	*/