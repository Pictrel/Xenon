#include <raylib.h>
#include <6502.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "font.h"

uint8_t diskdata[0x20000];

M6502 cpu;
int cpu_cyc;

bool irq_triggered;
int USE_LETTERBOX = 1;

/* Bubblegum 16 Palette (lospec)
Created by PineappleOnPizza
*/
/*Color cpal[16] = {
	(Color){0x16, 0x17, 0x1a, 0xFF},
	(Color){0x7f, 0x06, 0x22, 0xFF},
	(Color){0xd6, 0x24, 0x11, 0xFF},
	(Color){0xff, 0x84, 0x26, 0xFF},
	(Color){0xff, 0xd1, 0x00, 0xFF},
	(Color){0xfa, 0xfd, 0xff, 0xFF},
	(Color){0xff, 0x80, 0xa4, 0xFF},
	(Color){0xff, 0x26, 0x74, 0xFF},
	(Color){0x94, 0x21, 0x6a, 0xFF},
	(Color){0x43, 0x00, 0x67, 0xFF},
	(Color){0x23, 0x49, 0x75, 0xFF},
	(Color){0x68, 0xae, 0xd4, 0xFF},
	(Color){0xbf, 0xff, 0x3c, 0xFF},
	(Color){0x10, 0xd2, 0x75, 0xFF},
	(Color){0x00, 0x78, 0x99, 0xFF},
	(Color){0x00, 0x28, 0x59, 0xFF}
};

Color opal[16] = {
	(Color){0x16, 0x17, 0x1a, 0xFF},
	(Color){0x7f, 0x06, 0x22, 0xFF},
	(Color){0xd6, 0x24, 0x11, 0xFF},
	(Color){0xff, 0x84, 0x26, 0xFF},
	(Color){0xff, 0xd1, 0x00, 0xFF},
	(Color){0xfa, 0xfd, 0xff, 0xFF},
	(Color){0xff, 0x80, 0xa4, 0xFF},
	(Color){0xff, 0x26, 0x74, 0xFF},
	(Color){0x94, 0x21, 0x6a, 0xFF},
	(Color){0x43, 0x00, 0x67, 0xFF},
	(Color){0x23, 0x49, 0x75, 0xFF},
	(Color){0x68, 0xae, 0xd4, 0xFF},
	(Color){0xbf, 0xff, 0x3c, 0xFF},
	(Color){0x10, 0xd2, 0x75, 0xFF},
	(Color){0x00, 0x78, 0x99, 0xFF},
	(Color){0x00, 0x28, 0x59, 0xFF}
};*/

uint8_t ram[0xC000];
uint8_t vram[0x2000];
uint8_t bios[0x1000];
uint8_t bus = 0xFF;

uint8_t t_clock;

uint8_t IO_TCNT;
uint8_t IO_TCTL;
/*
--------
*/

uint8_t IO_TDIV;

uint8_t IO_VCTL;
/*
--------
*/

uint8_t IO_VSTA;
/*
--------
*/

uint8_t IO_VMX;
uint8_t IO_VMY;
uint8_t IO_VY;
uint8_t IO_VYC;
uint8_t IO_VWL;
uint8_t IO_VWR;

uint8_t IO_FCTL;
/*
--------

*/

//uint8_t IO_FSTA;
/*
Oeeeemmm

O: motor on
e: error code
	0 = OK
	1 = invalid command
	2 = read error
	3 = write error
	4 = no disk
	5 = busy
	8 = OS error
	
	E = motor not on
	F = undefined error
m: mode
	0 = off
	1 = idle
	4 = initializing
	5 = seeking
	6 = reading
	7 = writing
*/

uint8_t IO_FDAT;

uint8_t IO_FCMD;

uint8_t IO_ICTL;
/*
An NMI is fired every VBLANK to give time for the CPU to update data on screen.
As for the IRQ line, the CPU can select the desired interrupt source.

y-----ft

y: Select VYC interrupt 
t: Select Timer interrupt (when TCNT overflows)
f: Select Floppy interrupt
*/

bool fd_motor = false;
int fd_err = 0;
int fd_mode = 0;
int fd_timer = 0; //used internally to simulate FDD delay
int fd_pos   = 0;
int fd_arg;
int fd_idx;

Image fb_b;
Image fb_o;
Texture gpu_fb_b;
Texture gpu_fb_o;

#define SCREEN_W 200
#define SCREEN_H 150
#define VBLANK_SIZE 28

//#define GPU_DEBUG

#define GPU_ENABLE_SPRITES

typedef struct __attribute__((packed)) {
	uint8_t unused : 4;
	uint8_t hflip  : 1;
	uint8_t vflip  : 1;
	uint8_t pal    : 2;
	
	uint8_t x;
	uint8_t y;
	uint8_t c;
} OAMEnt;

typedef enum {
	FDD_NULL  = 0, /* no regs */
	FDD_SEEK  = 1, /* (FDAT = lba address), (no result), irq when done */
	FDD_READ  = 2, /* sets disk in mode 6, data returned periodically with IRQs*/
	FDD_WRITE = 3, /* sets disk in mode 7, data returned periodically with IRQs*/
	FDD_INIT  = 4, /* turns on motor, irq when done */
	FDD_TELL  = 5, /* read head position */
} FloppyCMD;

Color cpal(int i) {
	return (Color){((vram[0x400 + (i % 16)] & 0b11100000) >> 5 << 5),
	               ((vram[0x400 + (i % 16)] & 0b00011100) >> 2 << 5),
	               ((vram[0x400 + (i % 16)] & 0b00000011) >> 0 << 6), 0xff};
}

Color opal(int i) {
	return (Color){((vram[0x410 + (i % 16)] & 0b11100000) >> 5 << 5),
	               ((vram[0x410 + (i % 16)] & 0b00011100) >> 2 << 5),
	               ((vram[0x410 + (i % 16)] & 0b00000011) >> 0 << 6), 0xff};
}

int get_tile_value(int t, int x, int y) {
	return (
			  ((vram[0x1000 + t * 16 + y]     >> (7-x)) & 1) + 
			 (((vram[0x1000 + t * 16 + y + 8] >> (7-x)) & 1) << 1)
		);
}

void fdd_process() {
	if (fd_timer > 0) {
		fd_err = 5;
		m6502_irq(&cpu, TRUE);
		return;
	}
	
	switch (IO_FCMD) {
		case FDD_NULL: break;
		case FDD_SEEK:
			if (!fd_motor) {
				fd_err = 0xE;
				m6502_irq(&cpu, TRUE);
				break;
			}
			
			fd_err = 0;
			fd_mode = 5;
			fd_timer = 290 * abs(IO_FDAT - fd_pos);
			break;
		
		case FDD_INIT:
			fd_err = 0;
			fd_mode = 4;
			fd_timer = 4096;
			break;
		
		case FDD_TELL:
			IO_FDAT = fd_pos;
			fd_err = 0;
			break;
		
		case FDD_READ:
			fd_idx = 0;
			fd_mode = 6;
			
			break;
			
		default:
			fd_err = 1;
			break;
	}
}

void fdd_cycle() {
	if (fd_mode > 3) {
		fd_timer--;
		
		if (fd_timer == 0) {
			m6502_irq(&cpu, TRUE);
			switch (fd_mode) {
				case 4:
					fd_motor = true;
					break;
				
				case 5:
					fd_pos = fd_arg;
					break;
			}
			
			fd_mode = 1;
		}
		
		if (fd_mode == 6) {
			IO_FDAT = diskdata[fd_idx++];
			m6502_irq(&cpu, TRUE);
			if (fd_idx == 512) {
				fd_mode = 0;
			}
		}
	}
}

uint8_t joy1() {
	return (IsKeyDown(KEY_UP) << 0) |
	       (IsKeyDown(KEY_DOWN) << 1) |
	       (IsKeyDown(KEY_LEFT) << 2) |
	       (IsKeyDown(KEY_RIGHT) << 3) |
	       (IsKeyDown(KEY_Z) << 4) |
	       (IsKeyDown(KEY_X) << 5) |
	       (IsKeyDown(KEY_SPACE) << 6) |
	       (IsKeyDown(KEY_ENTER) << 7); 
}

uint8_t joy2() {
	return (IsKeyDown(KEY_I) << 0) |
	       (IsKeyDown(KEY_K) << 1) |
	       (IsKeyDown(KEY_J) << 2) |
	       (IsKeyDown(KEY_L) << 3) |
	       (IsKeyDown(KEY_A) << 4) |
	       (IsKeyDown(KEY_S) << 5) |
	       (IsKeyDown(KEY_Q) << 6) |
	       (IsKeyDown(KEY_W) << 7); 
}

uint8_t get_FSTA() {
	return ((fd_motor & 1) << 7) |
	       ((fd_err & 15) << 3) |
	       ((fd_mode & 7) << 0);
	
	//return fd_timer;
}

void cpu_iowrite(uint8_t addr, uint8_t val) {
	bus = val;
	
	switch (addr) {
		case 0x00: putchar(bus); fflush(stdout); break;
		
		//case 0x04: cpy_cyc = bus; break;
		//case 0x05: IO_TCNT = bus; break;
		case 0x06: IO_TCTL = bus; break;
		case 0x07: IO_TDIV = bus; break;
		
		case 0x10: IO_VCTL = bus; break;
		//case 0x11: IO_VSTA = bus; break;
		case 0x14: IO_VMX = bus; break;
		case 0x15: IO_VMY = bus; break;
		case 0x16: IO_VY = bus; break;
		case 0x17: IO_VYC = bus; break;
		case 0x18: IO_VWL = bus; break;
		case 0x19: IO_VWR = bus; break;
		
		case 0x80: IO_FCTL = bus; break;
		//case 0x81: IO_FSTA = bus; break;
		case 0x82: IO_FCMD = bus; fdd_process(); break;
		case 0x83: IO_FDAT = bus; break;
		
		case 0xFF: IO_ICTL = bus; break;
	}
	
	if (addr != 0x17) {
		//printf("W io%02x: %02x\n", addr, bus);
	}
}

uint8_t cpu_ioread(uint8_t addr) {
	
	switch (addr) {
		case 0x00: bus = getchar(); break;
		case 0x02: bus = joy1(); break;
		case 0x03: bus = joy2(); break;
		case 0x04: bus = cpu_cyc; break;
		case 0x05: bus = IO_TCNT; break;
		case 0x06: bus = IO_TCTL; break;
		case 0x07: bus = IO_TDIV; break;
		
		case 0x10: bus = IO_VCTL; break;
		case 0x11: bus = IO_VSTA; break;
		case 0x14: bus = IO_VMX; break;
		case 0x15: bus = IO_VMY; break;
		case 0x16: bus = IO_VY; break;
		case 0x17: bus = IO_VYC; break;
		case 0x18: bus = IO_VWL; break;
		case 0x19: bus = IO_VWR; break;
		
		case 0xFF: bus = IO_ICTL; break;
		
		case 0x80: bus = IO_FCTL; break;
		case 0x81: bus = get_FSTA(); break;
		case 0x82: bus = IO_FCMD; break;
		case 0x83: bus = IO_FDAT; break;
	}
	
	//printf("R io%02x: %02x\n", addr, bus);
	
	return bus;
}

zuint8 cpu_read(void *ctx, zuint16 addr) {
	if (     addr >= 0x0000 && addr <= 0xBFFF) bus = ram[addr % 0xC000];
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		bus = vram[addr % 0x2000];
		//printf("R %04x = %02x\n", addr, bus);
	}
	else if (addr >= 0xE000 && addr <= 0xEFFF) bus = cpu_ioread(addr & 0xFF);
	else if (addr >= 0xF000 && addr <= 0xFFFF) bus = bios[addr % 0x1000];
	
	return bus;
}

void cpu_write(void *ctx, zuint16 addr, zuint8 val) {
	bus = val;
	
	if (     addr >= 0x0000 && addr <= 0xBFFF) ram[addr % 0xC000] = bus;
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		vram[addr % 0x2000] = bus;
		//printf("W %04x = %02x\n", addr, bus);
	}
	else if (addr >= 0xE000 && addr <= 0xEFFF) cpu_iowrite(addr & 0xFF, bus);
	else if (addr >= 0xF000 && addr <= 0xFFFF) return; //u cant write to bios dummy
}

/* void DrawTile(int col, int x, int y, int chr) {
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			DrawRectangle((x + i) * 2,
			              (y + j) * 2, 2, 2, cpal[get_tile_value(chr, i, j) % 4 + (col % 4) * 4]);
		}
	}
}

void DrawSprite(int col, int x, int y, int chr) {
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			DrawRectangle((x + i) * 2,
			              (y + j) * 2, 2, 2, opal[get_tile_value(chr, i, j) % 4 + (col % 4) * 4]);
		}
	}
} */

Color bg_pixel(uint8_t x, uint8_t y) {
	x += IO_VMX;
	y += IO_VMY;

#ifdef GPU_DEBUG
	if (x == 0 || y == 0) return WHITE;
#endif

	uint8_t tilex = x / 8;
	uint8_t tiley = y / 8;
	uint8_t chrx = x % 8;
	uint8_t chry = y % 8;
	
	uint8_t tile = vram[0x800 + (tilex + tiley * 32)];
	uint8_t attr = vram[0xC00 + (tilex + tiley * 32)];
	
	return cpal(get_tile_value(tile, chrx, chry) % 4 + (attr % 4) * 4);
}

Color obj_pixel(uint8_t x, uint8_t y) {
	OAMEnt *oam = (OAMEnt*)vram;
	
	for (int i=0; i<64; i++) {
		if ((x - oam[i].x) < 8 && (x - oam[i].x) >= 0 &&
		    (y - oam[i].y) < 8 && (y - oam[i].y) >= 0) {
			
			int v = get_tile_value(oam[i].c, x - oam[i].x, y - oam[i].y);
			
			return v ? opal(v + oam[i].pal * 4) : BLANK;
		}
	}
	
	return BLANK;
}

void render_pixel(int x, int y) {
	
	
	ImageDrawPixel(&fb_b, x, y, bg_pixel(x, y));
	
#ifdef GPU_ENABLE_SPRITES
	Color op = obj_pixel(x, y); 
	ImageDrawPixel(&fb_o, x, y, op);
#endif

#ifdef GPU_DEBUG
	
	if (IO_VY == IO_VYC && x < 8) {
		//printf("%02x %02x\n", IO_VY, IO_VYC);
		ImageDrawPixel(&fb_b, x, y, YELLOW);
	}
#endif
}

void render_scanline(int y) {
	for (int x=0; x<200; x++) {
		render_pixel(x, y);
	} 
	
	fdd_cycle();
}

void vyc_int() {
	if (IO_VY == IO_VYC) {
		m6502_irq(&cpu, TRUE);
		irq_triggered = true;
		/*printf("%02x %02x %c%c\n", IO_VY, IO_VYC, (IO_VY > SCREEN_H) ? 'V' : 'D',
		                                          irq_triggered ? '!' : ' ');*/
	}
	
}

void handle_scn() {
	if (IO_ICTL & 0b10000000) vyc_int();
}

void handle_tim() {
	
}

void update(void) {
	for (int y=0; y<SCREEN_H + VBLANK_SIZE / 4; y++) {
		if (y == SCREEN_H) {
			m6502_nmi(&cpu);
		}
		
		irq_triggered = false;
		
		IO_VY = y;
		handle_scn();
		
		render_scanline(IO_VY);
		
		cpu_cyc += m6502_run(&cpu, 1);
		m6502_irq(&cpu, FALSE);
		cpu_cyc += m6502_run(&cpu, 64);
		fdd_cycle();
		cpu_cyc += m6502_run(&cpu, 64);
		fdd_cycle();
		cpu_cyc += m6502_run(&cpu, 64);
		fdd_cycle();
		cpu_cyc += m6502_run(&cpu, 64);
		fdd_cycle();
		
		//printf("C%d\n", cpu_cyc);
		
	}
}

/*void DrawChar(int col, int x, int y, int attr) {
	
	for (int i=0; i<8; i++) {
		for (int j=0; j<12; j++) {
			if (attr & 0x80) {
				DrawRectangle((x + i*2) * 2, (y + j*2) * 2, 4, 4,
					((font_8x12[(attr % 128) * 12 + j] >> (7-i)) & 1) ? opal[col % 16] : BLANK
				);
			} else {
				DrawRectangle((x + i) * 2, (y + j) * 2, 2, 2,
					((font_8x12[(attr & 0x7f) * 12 + j] >> (7-i)) & 1) ? opal[col % 16] : BLANK
				);
			}
		}
	}
}*/

/* void DrawBackground() {
	for (int x=0; x<64; x++) {
		for (int y=0; y<64; y++) {
			int eff_x = x * 8 - IO_VMX; //effective X coord
			int eff_y = y * 8 - IO_VMY; //effective Y coord
			
			if ((eff_x > -8) && (eff_y > -8) &&
			    (eff_x < 192+8) && (eff_y < 144+8)) {
				uint8_t tile = vram[0x800 + (x % 32) + (y % 32) * 32];
				uint8_t attr = vram[0xC00 + (x % 32) + (y % 32) * 32];
				
				DrawTile(attr & 0b1111, eff_x, eff_y, tile);
			}
		}
	}
} */

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
	
	/* ClearBackground(BLACK);
	
	VRAMEnt *gfx = (VRAMEnt*)vram;
	
	
	printf("\n");
	for (int i=0; i<1024; i++) {
		if ((i % 16) == 0) printf("\n");
		printf("%02x ", vram[i]);
	}
	
	
	DrawBackground();
	
	for (int i=0; i<256; i++) {
		switch (gfx[i].type) {
			case 0x00: break;
			case 0x01: break;
			case 0x02: break;
			case 0x03: break;
			case 0x04: break;
			case 0x05: break;
			case 0x06: DrawTile(gfx[i].col, gfx[i].x, gfx[i].y, gfx[i].attr); break;
			case 0x07: DrawChar(gfx[i].col, gfx[i].x, gfx[i].y, gfx[i].attr); break;
			case 0x08: break;
			case 0x09: break;
			case 0x0A: break;
			case 0x0B: break;
			case 0x0C: break;
			case 0x0D: break;
			case 0x0E: break;
			case 0x0F: break;
		}
	} */
	
	DrawFPS(0, 0); 
} 

int main(int argc, char **argv) {
	FILE *fp = fopen("bios.bin", "r");
	if (!fp) fp = fopen("bin/bios.bin", "r");
	if (!fp) fp = fopen("/usr/share/xenon/bios.bin", "r");
	
	if (!fp) {
		printf("Unable to load BIOS file\n");
		return 1;
	}
	
	if (argc < 2) {
		printf("usage: xenon <disk>\n");
		return 1;
	}
	
	fread(bios, 1, 0x1000, fp);
	
	fclose(fp);
	fp = fopen(argv[1], "r");
	if (!fp) {
		perror(argv[1]);
		return 1;
	}
	
	fread(diskdata, 1, 0x20000, fp);
	
	SetTraceLogLevel(10);
	InitWindow(SCREEN_W * 2, SCREEN_H * 2, "xenon");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	
	SetWindowMinSize(SCREEN_W, SCREEN_H);
	
	cpu.read = cpu_read;
	cpu.write = cpu_write;
	
	m6502_power(&cpu, TRUE);
	cpu.state.pc = cpu_read(NULL, 0xfffc) | (cpu_read(NULL, 0xfffd) << 8);
	
	fb_b = GenImageColor(SCREEN_W, SCREEN_H, BLANK);
	fb_o = GenImageColor(SCREEN_W, SCREEN_H, BLANK);
	
	while (!WindowShouldClose()) {
		BeginDrawing();
		update();
		draw();
		EndDrawing();
	}
	
	CloseWindow();
	
	return 0;
}
