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
	H_ASM = 2
} hi_sel = H_CONS;
uint16_t mem_sel = 0xF000;
uint16_t prg_sel = 0xF000;
uint16_t mem_pointer = 0xF000;
uint16_t prg_pointer = 0xF000;
bool unbound = false;

extern bool irq;
extern bool nmi;

Color rgb332(uint8_t n);

bool handle_scn(void);
void render_pixel(int x, int y);
void render_scanline(int y);

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

char *get_inst_name(uint16_t addr) {
	switch (opcodes[cpu_read(NULL, addr)].m) {
		case NONE: return TextFormat("---");
		case ACCU: return TextFormat("%3s A",            opcodes[cpu_read(NULL, addr)].o);
		
		
		case ABSL: return TextFormat("%3s $%02x%02x",    opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+2),
		                                                         cpu_read(NULL, addr+1));
		case ABSX: return TextFormat("%3s $%02x%02x,X",  opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+2),
		                                                         cpu_read(NULL, addr+1));
		case ABSY: return TextFormat("%3s $%02x%02x,Y",  opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+2),
		                                                         cpu_read(NULL, addr+1));
		
		
		case IMMD: return TextFormat("%3s #$%02x",        opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+1));
		
		case IMPL: return TextFormat("%3s",              opcodes[cpu_read(NULL, addr)].o);
		
		case INDI: return TextFormat("%3s ($%02x%02x)",  opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+2),
		                                                         cpu_read(NULL, addr+1));
		                                                         
		case XIND: return TextFormat("%3s ($%02x%02x,X)",opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+2));

		case INDY: return TextFormat("%3s ($%02x%02x),Y",opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+2));

		case RELA: return TextFormat("%3s $%02x",        opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+1));
		
		case ZPAG: return TextFormat("%3s $%02x",        opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+1));
		case ZPGX: return TextFormat("%3s $%02x,X",      opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+1));
		case ZPGY: return TextFormat("%3s $%02x,Y",      opcodes[cpu_read(NULL, addr)].o,
		                                                         cpu_read(NULL, addr+1));
	}
}

void debug_update() {
	ClearWindowState(FLAG_WINDOW_RESIZABLE);
	
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
	
	if (cpu_running) {
		for (int i=0; i<cpu_speed; i++) {
			sys_step();
		}
	}
	
	if (IsKeyPressed(KEY_A) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = true, cpu_speed = 1;
	if (IsKeyPressed(KEY_S) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = false;
	if (IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = true, cpu_speed = 166420;
	if (IsKeyPressed(KEY_H) && IsKeyDown(KEY_LEFT_CONTROL)) cpu_running = true, cpu_speed = 1060;
	
	if (IsKeyPressed(KEY_D) && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_ASM;
	if (IsKeyPressed(KEY_M) && IsKeyDown(KEY_LEFT_CONTROL)) hi_sel = H_MEM;
	if (IsKeyPressed(KEY_U) && IsKeyDown(KEY_LEFT_CONTROL)) {
		unbound = !unbound;
		SetTargetFPS(unbound ? 0 : 60);
	}
	
	if (hi_sel == H_MEM) {
		if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))       mem_sel -= IsKeyDown(KEY_LEFT_CONTROL) ? 0x80 : 0x08;
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))   mem_sel += IsKeyDown(KEY_LEFT_CONTROL) ? 0x80 : 0x08;
		if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))   mem_sel--;
		if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) mem_sel++;
		
		if (mem_sel < (mem_pointer))        mem_pointer = (mem_sel & 0xFFF8);
		if (mem_sel > (mem_pointer + 0xA0)) mem_pointer = (mem_sel & 0xFFF8) - 0xA0;
	}
	
	if (hi_sel == H_ASM) {
		if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))       prg_sel--;
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))   prg_sel += get_inst_width(prg_sel);
	}
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
	int offset = 0;
	for (int y=0; y<32; y++) {
		DrawStringMonospace(TextFormat("%04X", prg_pointer + offset), 8 + 7, 168 + y * 8 + 8, 1, 0, WHITE);
		DrawStringMonospace(TextFormat("%04X", prg_pointer + offset), 9 + 7, 168 + y * 8 + 8, 1, 0, WHITE);
		
		DrawStringMonospace(TextFormat("%3s", get_inst_name(prg_pointer + offset)), 9 + 7 * 6, 168 + y * 8 + 8, 1, 0, WHITE);
		
		offset += get_inst_width(prg_pointer + offset);
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
}

void debug_draw() {
	//GuiEnable();
	
	ClearBackground(BLACK);
	
	if (IsTextureValid(gpu_fb_b)) UnloadTexture(gpu_fb_b);
		gpu_fb_b = LoadTextureFromImage(fb_b);
	
	if (IsTextureValid(gpu_fb_o)) UnloadTexture(gpu_fb_o);
	gpu_fb_o = LoadTextureFromImage(fb_o);
	
	// Display 
	DrawRectangleLines(8,  8, 200+4, 150+4, rgb332(0xff));
	DrawRectangleLines(9,  9, 200+2, 150+2, rgb332(0xff));
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
	DrawStringMonospace(TextFormat("NMI"),                  220,          10 + 16 * 8, 2, 0, nmi?GOLD:WHITE);
	DrawStringMonospace(TextFormat("IRQ"),                  220 + 14 * 4, 10 + 16 * 8, 2, 0, irq?GOLD:WHITE);
	DrawStringMonospace(TextFormat("%s", get_state_name()), 220 + 14 * 8, 10 + 16 * 8, 2, 0, WHITE);
	
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