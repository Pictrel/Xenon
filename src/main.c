#include <raylib.h>
#include <6502.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "font.h"

M6502 cpu;
int cpu_cyc;

int USE_LETTERBOX = 1;

/* Bubblegum 16 Palette (lospec)
Created by PineappleOnPizza
*/
Color cpal[16] = {
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
};

uint8_t ram[0xC000];
uint8_t vram[0x2000];
uint8_t bios[0x1000];
uint8_t bus = 0xFF;

uint8_t IO_TCNT;
uint8_t IO_TCTL;
uint8_t IO_TDIV;

uint8_t IO_VCTL;
uint8_t IO_VSTA;
uint8_t IO_VMX;
uint8_t IO_VMY;
uint8_t IO_VY;
uint8_t IO_VYC;
uint8_t IO_VWL;
uint8_t IO_VWR;

uint8_t IO_ICTL;

Image fb;
Texture gpu_fb;

#define SCREEN_W 200
#define SCREEN_H 150
#define VBLANK_SIZE 28

#define GPU_DEBUG

typedef struct __attribute__((packed)) {
	uint8_t col    : 4;
	uint8_t type   : 4;
	
	uint8_t x;
	uint8_t y;
	uint8_t attr;
} OAMEnt;

int get_tile_value(int t, int x, int y) {
	return (
			  ((vram[0x1000 + t * 16 + y]     >> (7-x)) & 1) + 
			 (((vram[0x1000 + t * 16 + y + 8] >> (7-x)) & 1) << 1)
		);
}

void cpu_iowrite(uint8_t addr, uint8_t val) {
	bus = val;
	
	switch (addr) {
		case 0x00: putchar(bus); fflush(stdout); break;
		
		//case 0x04: cpy_cyc = bus; break;
		case 0x05: IO_TCNT = bus; break;
		case 0x06: IO_TCTL = bus; break;
		case 0x07: IO_TDIV = bus; break;
		
		case 0x10: IO_VCTL = bus; break;
		case 0x11: IO_VSTA = bus; break;
		case 0x14: IO_VMX = bus; break;
		case 0x15: IO_VMY = bus; break;
		case 0x16: IO_VY = bus; break;
		case 0x17: IO_VYC = bus; break;
		case 0x18: IO_VWL = bus; break;
		case 0x19: IO_VWR = bus; break;
	}
}

uint8_t cpu_ioread(uint8_t addr) {
	
	switch (addr) {
		case 0x00: bus = getchar(); break;
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
	}
	
	printf("R io%02x: %02x\n", addr, bus);
	
	return bus;
}

zuint8 cpu_read(void *ctx, zuint16 addr) {
	if (     addr >= 0x0000 && addr <= 0xBFFF) bus = ram[addr % 0xC000];
	else if (addr >= 0xC000 && addr <= 0xDFFF) bus = vram[addr % 0x2000];
	else if (addr >= 0xE000 && addr <= 0xEFFF) bus = cpu_ioread(addr & 0xFF);
	else if (addr >= 0xF000 && addr <= 0xFFFF) bus = bios[addr % 0x1000];
	
	return bus;
}

void cpu_write(void *ctx, zuint16 addr, zuint8 val) {
	bus = val;
	
	if (     addr >= 0x0000 && addr <= 0xBFFF) ram[addr % 0xC000] = bus;
	else if (addr >= 0xC000 && addr <= 0xDFFF) vram[addr % 0x2000] = bus;
	else if (addr >= 0xE000 && addr <= 0xEFFF) cpu_iowrite(addr & 0xFF, bus);
	else if (addr >= 0xF000 && addr <= 0xFFFF) ; //u cant write to bios dummy
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
	
	return cpal[get_tile_value(tile, chrx, chry) % 4];
}

void render_pixel(int x, int y) {
	ImageDrawRectangle(&fb, x, y, 1, 1, bg_pixel(x, y));
}

void render_scanline(int y) {
	
	
	for (int x=0; x<256; x++) {
		render_pixel(x, y);
	} 
}

void update() {
	for (int y=0; y<SCREEN_H * 4 + VBLANK_SIZE; y++) {
		if (y == SCREEN_H * 4) {
			m6502_nmi(&cpu);
		}
		
		IO_VY = y / 4;
		
		cpu_cyc += m6502_run(&cpu, 264);
		
		//printf("C%d\n", cpu_cyc);
		
		render_scanline(IO_VY);
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

void draw() {
	ClearBackground(WHITE);
	if (IsTextureValid(gpu_fb)) UnloadTexture(gpu_fb);
	gpu_fb = LoadTextureFromImage(fb);
	
	int SCALE = fmin(GetRenderWidth() / SCREEN_W,
	                 GetRenderHeight() / SCREEN_H);
	
	DrawTexturePro(gpu_fb, 
		(Rectangle){0, 0, SCREEN_W, SCREEN_H},
		(Rectangle){0, 0, GetRenderWidth(), GetRenderHeight()},
		(Vector2){0, 0}, 0.0f, WHITE);
	
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

int main() {
	FILE *fp = fopen("bios.bin", "r");
	if (!fp) fp = fopen("bin/bios.bin", "r");
	if (!fp) fp = fopen("/usr/share/r6502/bios.bin", "r");
	
	if (!fp) {
		printf("Unable to load BIOS file\n");
		return 1;
	}
	
	SetTraceLogLevel(10);
	
	fread(bios, 1, 0x1000, fp);
	
	InitWindow(SCREEN_W * 2, SCREEN_H * 2, "xenon");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	//SetTargetFPS(0);
	
	SetWindowMinSize(SCREEN_W, SCREEN_H);
	
	cpu.read = cpu_read;
	cpu.write = cpu_write;
	
	m6502_power(&cpu, TRUE);
	cpu.state.pc = cpu_read(NULL, 0xfffc) | (cpu_read(NULL, 0xfffd) << 8);
	
	fb = GenImageColor(SCREEN_W, SCREEN_H, BLANK);
	
	while (!WindowShouldClose()) {
		BeginDrawing();
		update();
		draw();
		EndDrawing();
	}
	
	CloseWindow();
	
	return 0;
}
