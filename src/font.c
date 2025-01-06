#include "antarex.h"
#include <raylib.h>
#include <stdio.h>

int MeasureChar(uint8_t c, int scale) {
	return ((antarex[c].end - antarex[c].start) + 2) * scale;
}

//returns width of character
int DrawChar(uint8_t c, int x, int y, int scale, Color col) {
	if (c < ANTAREX_START) return 0;
	
	c -= ANTAREX_START;
	
	for (int i=antarex[c].start; i<=antarex[c].end; i++) {
		for (int j=0; j<ANTAREX_HEIGHT; j++) {
			DrawRectangle(x + i * scale,
			              y + j * scale, scale, scale, ((antarex[(uint8_t)c].bitmap[i] >> (ANTAREX_HEIGHT-1)-j) & 1) ? col : BLANK);
		}
	}
	
	return MeasureChar(c, scale);
}

//returns width of character
int DrawCharMonospace(uint8_t c, int x, int y, int scale, Color col) {
	if (c < ANTAREX_START) return 0;
	
	c -= ANTAREX_START;
	
	for (int i=0; i<ANTAREX_WIDTH; i++) {
		for (int j=0; j<ANTAREX_HEIGHT; j++) {
			DrawRectangle(x + i * scale,
			              y + j * scale, scale, scale, ((antarex[(uint8_t)c].bitmap[i] >> (ANTAREX_HEIGHT-1)-j) & 1) ? col : BLANK);
		}
	}
	
	return ANTAREX_WIDTH * scale;
}

int MeasureString(char *s, int scale, int spacing) {
	int o = 0;
	
	for (int i=0; s[i]; i++) {
		o += MeasureChar(s[i], scale) + 0 * scale;
	}
	
	return o;
}

int DrawString(uint8_t *s, int x, int y, int scale, int spacing, Color col) {
	int o = 0;
	
	for (int i=0; s[i]; i++) {
		o += DrawChar(s[i], x+o, y, scale, col) + 0 * scale;
	}
	
	o = 0;
	
	return o;
}

int DrawStringMonospace(uint8_t *s, int x, int y, int scale, int spacing, Color col) {
	int o = 0;
	
	for (int i=0; s[i]; i++) {
		o += DrawCharMonospace(s[i], x+o, y, scale, col) + spacing;
	}
	
	return o;
}






int ImageDrawChar(            Image *img, uint8_t  c, int x, int y, int scale,              Color col) {
	if (c < ANTAREX_START) return 0;
	
	c -= ANTAREX_START;
	
	for (int i=antarex[c].start; i<=antarex[c].end; i++) {
		for (int j=0; j<ANTAREX_HEIGHT; j++) {
			if (((antarex[(uint8_t)c].bitmap[i] >> (ANTAREX_HEIGHT-1)-j) & 1))
				ImageDrawRectangle(img,
			              x + i * scale,
			              y + j * scale, scale, scale, col);
		}
	}
	
	return MeasureChar(c, scale);
}

int ImageDrawCharMonospace(   Image *img, uint8_t  c, int x, int y, int scale,              Color col) {
	if (c < ANTAREX_START) return 0;
	
	c -= ANTAREX_START;
	
	for (int i=0; i<ANTAREX_WIDTH; i++) {
		for (int j=0; j<ANTAREX_HEIGHT; j++) {
			if (((antarex[(uint8_t)c].bitmap[i] >> (ANTAREX_HEIGHT-1)-j) & 1))
				ImageDrawRectangle(img,
			              x + ((ANTAREX_WIDTH / 2) - (antarex[(uint8_t)c].end-
			                                       antarex[(uint8_t)c].start) / 2 + i) * scale,
			              y + (j) * scale, scale, scale, col);
		}
	}
	
	return ANTAREX_WIDTH * scale;
}

int ImageDrawString(          Image *img, uint8_t *s, int x, int y, int scale, int spacing, Color col) {
	int o = 0;
	
	for (int i=0; s[i]; i++) {
		o += ImageDrawChar(img, s[i], x+o, y, scale, col) + 0 * scale;
	}
	
	o = 0;
	
	return o;
}

int ImageDrawStringMonospace( Image *img, uint8_t *s, int x, int y, int scale, int spacing, Color col) {
	int o = 0;
	
	for (int i=0; s[i]; i++) {
		o += ImageDrawCharMonospace(img, s[i], x+o, y, scale, col) + spacing;
	}
	
	return o;
}
