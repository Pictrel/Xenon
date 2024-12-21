#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *filename;

uint8_t tileset[256 * 16] = {};

uint8_t saved_tile[16];

#define cur_col  PROP[1]

int PROP[256];
int cur_tile = 0;
int cur_x = 0;
int cur_y = 0;

Font font;
Font bold;

/*
Ice Cream GB (lospec)
Created by Kerrie Lake
*/
const Color pal[4] = {
	(Color){0x7c, 0x3f, 0x58, 0xff},
	(Color){0xeb, 0x6b, 0x6f, 0xff},
	(Color){0xf9, 0xa8, 0x75, 0xff},
	(Color){0xff, 0xf6, 0xd3, 0xff}
};

bool mouse_hovers(int x, int y, int w, int h) {
	return (GetMouseX() > x) && (GetMouseX() < x + w) &&
			(GetMouseY() > y) && (GetMouseY() < y + h);
}

int get_tile_value(int t, int x, int y) {
	return (
			  ((tileset[t * 16 + y]     >> (7-x)) & 1) + 
			 (((tileset[t * 16 + y + 8] >> (7-x)) & 1) << 1)
		);
}

void set_tile_value(int t, int x, int y, int v) {
	tileset[t * 16 + y]     &= (~(1<<(7-x)));
	tileset[t * 16 + y + 8] &= (~(1<<(7-x)));
	
	tileset[t * 16 + y]     |= (((v & 1) >> 0) << (7-x));
	tileset[t * 16 + y + 8] |= (((v & 2) >> 1) << (7-x));
}

void render_tile(int t, int x, int y, int scale, int spacing) {
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			DrawRectangle(x + i * scale,
			              y + j * scale, scale - spacing,
			                             scale - spacing,
			                             pal[get_tile_value(t, i, j)]);
		}
	}
}

void render_tile_c(int *c, int x, int y, int scale, int spacing) {
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			DrawRectangle(x + i * scale,
			              y + j * scale, scale - spacing,
			                             scale - spacing,
			                             pal[(
											  ((c[y]     >> (7-x)) & 1) + 
											 (((c[y + 8] >> (7-x)) & 1) << 1)
										)]
				);
		}
	}
}

void fliph_tile(int t) {
	for (int i=0; i<16; i++) {
		uint8_t tile = 0;
		
		for (int j=0; j<8; j++) {
			tile <<= 1;
			
			tile |= (tileset[t * 16 + i] >> (j)) & 1;
			
			
		}
		
		tileset[t * 16 + i] = tile;
	}
}

void flipv_tile(int t) {
	uint8_t tile[16];
	
	for (int i=0; i<8; i++) {
		tile[i]   = tileset[t * 16 + (7-i)];
		tile[i+8] = tileset[t * 16 + (7-i+8)];
	}
	
	memcpy(&tileset[t * 16], tile, 16);
}

void move_tile_curs(int x, int y) {
	printf("%d %d\n", x, y);
	cur_x = (cur_x + x) % 8;
	cur_y = (cur_y + y) % 8;
	
	if (cur_x < 0) cur_x = 7;
	if (cur_y < 0) cur_y = 7;
}

void move_map_curs(int x, int y) {
	printf("%d %d\n", x, y);
	
	cur_tile = (cur_tile + (x + y * 16)) % 256;
	
	if (cur_tile < 0) cur_tile =  255;
}

void fill_tile(int t, int c) {
	for (int x=0; x<8; x++) {
		for (int y=0; y<8; y++) {
			set_tile_value(t, x, y, c);
		}
	}
}

void replace_tile(int t, int from, int to) {
	for (int x=0; x<8; x++) {
		for (int y=0; y<8; y++) {
			if (get_tile_value(t, x, y) == from )set_tile_value(t, x, y, to);
		}
	}
}

void save() {
	if (!filename) filename = "tileset.chr";
	
	FILE *fp = fopen(filename, "w");
	
	if (!fp) {
		perror("Unable to write file");
		return;
	}
	
	fwrite(tileset, 1, sizeof tileset, fp);
	fclose(fp);
}

void load() {
	if (!filename) {
		printf("No file specified.\n");
		return;
	}
	
	FILE *fp = fopen(filename, "r");
	
	if (!fp) {
		perror("Unable to read file");
		return;
	}
	
	fread(tileset, 1, sizeof tileset, fp);
	fclose(fp);
}

void process_key(int key) {

	if (IsKeyDown(KEY_LEFT_CONTROL)) switch (key) {
		
		case KEY_S: save();
		case KEY_L: load();
		
	} else switch (key) {
		// Tile Movement
		case KEY_UP:    move_tile_curs(0, -1); break;
		case KEY_DOWN:  move_tile_curs(0, 1); break;
		case KEY_LEFT:  move_tile_curs(-1, 0); break;
		case KEY_RIGHT: move_tile_curs(1, 0); break;
		case KEY_I:     move_map_curs(0, -1); break;
		case KEY_K:     move_map_curs(0, 1); break;
		case KEY_J:     move_map_curs(-1, 0); break;
		case KEY_L:     move_map_curs(1, 0); break;
		
		// Palette
		case KEY_ONE:   cur_col = 0; break;
		case KEY_TWO:   cur_col = 1; break;
		case KEY_THREE: cur_col = 2; break;
		case KEY_FOUR:  cur_col = 3; break;
		case KEY_A: cur_col--; if (cur_col < 0) cur_col = 3; break;
		case KEY_S: cur_col++; cur_col %= 4; break;
		
		
		// Tools
		case KEY_Q: set_tile_value(cur_tile, cur_x, cur_y, cur_col); break;
		case KEY_W: set_tile_value(cur_tile, cur_x, cur_y, 0); break;
		case KEY_E: replace_tile(cur_tile, get_tile_value(cur_tile, cur_x, cur_y), cur_col); break;
		case KEY_T: fliph_tile(cur_tile); break;
		case KEY_Y: flipv_tile(cur_tile); break;
		
		case KEY_DELETE: fill_tile(cur_tile, 0); break;
		
		// Copy + Paste
		case KEY_C: memcpy(saved_tile, tileset + cur_tile * 16, 16); break;
		case KEY_V: memcpy(tileset + cur_tile * 16, saved_tile, 16); break;
		
		// Save and load
		
	}
}

void update() {
	int key;
	
	while (key = GetKeyPressed()) {
		printf("%04x\n", key);
		process_key(key);
	}
}

void draw() {
	ClearBackground(BLACK);
	DrawTextEx(bold, "XGE", (Vector2){20, 20}, 40, 5, WHITE);
	
	render_tile(cur_tile, 20, 60, 24, 3);
	
	//call_click_interaction(20, 60, 24, 3, click_on_tile);
	
	for (int i=0; i<4; i++) {
		DrawRectangle(192+40, 60 + i * (192 / 4), 60, 192 / 4, pal[i]);
		DrawTextEx(font, TextFormat("%d",i+1), (Vector2){192+40+75, 60 + i * (192 / 4) + 15}, 20, 0, WHITE);
	}
	
	//DrawText("O", 256, 30, 20, WHITE);
	//DrawText("L", 256, 260, 20, WHITE);
	
	for (int i=0; i<4; i++) {
		if (cur_col == i) {
			DrawRectangleLinesEx((Rectangle){192+40-5, 60 + i * (192 / 4) - 5, 60 + 10, 192 / 4 + 10}, 5, WHITE);
		}
	}
	
	for (int x=0; x<16; x++) {
		for (int y=0; y<16; y++) {
			render_tile(x + y * 16, 340+x*16, 60+y*16, 2, 0);
		}
	}
	
	for (int i=0; i<16; i++) {
		DrawTextEx(bold, TextFormat("%X", i), (Vector2){345 + i * 16, 45}, 10, 0, GRAY);
		DrawTextEx(bold, TextFormat("%X", i), (Vector2){345 - 16, 45 + i * 16 + 16}, 10, 0, GRAY);
	}
	
	DrawRectangleLinesEx((Rectangle){20 + cur_x * 24,
	                                 60 + cur_y * 24, 22, 22}, 2, WHITE);
	
	DrawRectangleLines((cur_tile % 16) * 16 + 340,
	                   (cur_tile / 16) * 16 + 60,
	                   16, 16, WHITE);
	
	DrawText("Q: Pencil\nW: Eraser", 30, 262, 20, WHITE);
	
	for (int i=0; i<16; i++) {
		DrawText(TextFormat("$%02X,", tileset[cur_tile * 16 + i]), 20+i*25, 340, 10, WHITE);
	}
	
}

int main(int argc, char **argv) {
	InitWindow(640, 480, "XGE");
	
	SetTargetFPS(60);
	
	if (argc > 1) filename = strdup(argv[1]);
	load();
	
	while (!WindowShouldClose()) {
		BeginDrawing();
		update();
		draw();
		EndDrawing();
	}
}