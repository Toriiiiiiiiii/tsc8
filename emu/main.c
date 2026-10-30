#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include <raylib.h>

#include "MCS6502.h"

#define CLOCK_FREQ 4000000
#define NANOSECS   1000000000

#define RAM_SIZE 0x8000
#define ROM_SIZE 0x2000
#define RAM_BASE 0x0000
#define ROM_BASE 0xe000

#define KEYB_BUFSIZE 256

static uint8_t RAM[RAM_SIZE] = {0};
static uint8_t ROM[ROM_SIZE] = {0};
static MCS6502ExecutionContext context;

static int running = 1;
static volatile int cpuLock = 0;
static int enableCPU = 0;

static uint8_t keyb[KEYB_BUFSIZE] = {0};
static uint8_t keyb_front = 0;
static uint8_t keyb_back = 0;

#define NLINES 21
#define NCOLS  71

static char tty[NLINES][NCOLS+1] = {{0}};
static int line = 0;
static int col = 0;

uint8_t readByte(uint16_t addr, void *ctx) {
	if(RAM_BASE <= addr && addr < RAM_BASE+RAM_SIZE) 
		return RAM[addr];

	if(ROM_BASE <= addr && addr < ROM_BASE+ROM_SIZE)
		return ROM[addr & 0b0001111111111111];

	if(addr == 0x8001) { 
		uint8_t key = keyb[keyb_front];
		if(key != 0) keyb_front++;
		return key;
	}

	return 0xff;
}


void advanceTTY() {
	line++;
	col = 0;
	if(line >= NLINES) {
		for(int i = 0; i < NLINES-1; ++i) {
			memcpy(tty[i], tty[i+1], NCOLS);
		}
		line--;
		memset(tty[line], 0, NCOLS+1);
	}
}


void writeByte(uint16_t addr, uint8_t data, void *ctx) {
	if(RAM_BASE < addr && addr < RAM_BASE+RAM_SIZE)
		RAM[addr] = data;

	if(addr == 0x8000 && data != 0) {
		//printf("%c", data);
		//fflush(stdout);

		if(data == '\n') {
			advanceTTY();
			return;
		} else if(data == '\r') {
			col = 0;
			return;
		} else if(data == '\b') {
			col--;
			if(col < 0) col = 0;
			return;
		}

		tty[line][col++] = (char)data;
		
		if(col >= NCOLS) advanceTTY();
	}
}


void *render(void *args) {
	SetTraceLogLevel(LOG_ERROR);
	InitWindow(640, 480, "TSC-8 Emulator");
	SetTargetFPS(60);

	Texture bgTex = LoadTexture("asset/emu-bg.png");
	Texture ledOff = LoadTexture("asset/emu-ledoff.png");
	Texture ledOn = LoadTexture("asset/emu-ledon.png");

	Font fnt = LoadFont("asset/departure-mono.otf");

	uint16 addr = 0;
	uint8  data = 0;

	const int buttonSize = 16;

	const int enableX = 22*16;
	const int enableY = 16;

	const int resetX  = 17*16;
	const int resetY  = 3*16;

	const int statusX = 30*16;
	const int statusY = enableY + 16;

	const int addressX = 2*16;
	const int addressY = 7*16;
	const int dataX = addressX + 16*16 + 32;
	const int dataY = addressY;

	const int writeX = dataX + 8*16 + 16;
	const int writeY = dataY;

	const int ttyY = 9*16;

	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(GRAY);

		DrawTexturePro(bgTex, (Rectangle){0,0,320,240}, (Rectangle){0,0,640,480}, (Vector2){0,0}, 0.0f, WHITE);

		DrawTexturePro(enableCPU? ledOn : ledOff, (Rectangle){0,0,8,8}, (Rectangle){enableX,enableY,16,16}, (Vector2){0,0}, 0, WHITE);
		DrawTexturePro(ledOff, (Rectangle){0,0,8,8}, (Rectangle){resetX,resetY,16,16}, (Vector2){0,0}, 0, WHITE);
		DrawTexturePro(ledOff, (Rectangle){0,0,8,8}, (Rectangle){writeX,writeY,16,16}, (Vector2){0,0}, 0, WHITE);

		// Draw TTY
		for(int i = 0; i < NLINES; ++i) {
			DrawTextEx(fnt, tty[i], (Vector2){0, (float)(ttyY+16*i)}, 16, 1, WHITE);
		}
		const int cw = 640/NCOLS;
		DrawRectangle((col*cw), ttyY + (line*16) + 14, cw, 2, WHITE);

		if(enableCPU) {
			addr = context.pc;
			data = readByte(addr, NULL);

			if(IsKeyPressed(KEY_ENTER)) {
				keyb[keyb_back++] = '\n';
			}

			if(IsKeyPressed(KEY_BACKSPACE)) {
				keyb[keyb_back++] = '\b';
			}

			int key = GetCharPressed();
			while(key > 0) {
				keyb[keyb_back++] = key;
				key = GetCharPressed();
			}
		}

		for(int i = 0; i < 16; ++i) {
			int x = addressX + (16*i);
			int y = addressY;

			int bit = (addr >> (15-i)) & 1;
			DrawTexturePro(bit? ledOn : ledOff, (Rectangle){0,0,8,8}, (Rectangle){x,y,16,16}, (Vector2){0,0}, 0, WHITE);
		}	

		for(int i = 0; i < 8; ++i) {
			int x = dataX + (16*i);
			int y = dataY;

			int bit = (data >> (7-i)) & 1;
			DrawTexturePro(bit? ledOn : ledOff, (Rectangle){0,0,8,8}, (Rectangle){x,y,16,16}, (Vector2){0,0}, 0, WHITE);
		}	
		for(int i = 0; i < 8; ++i) {
			int x = statusX + (16*i);
			int y = statusY;

			int bit = (context.p >> (7-i)) & 1;
			DrawTexturePro(bit? ledOn : ledOff, (Rectangle){0,0,8,8}, (Rectangle){x,y,16,16}, (Vector2){0,0}, 0, WHITE);
		}	

		EndDrawing();

		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			int x = GetMouseX();
			int y = GetMouseY();

			if(x >= enableX && x < enableX+buttonSize && y >= enableY && y < enableY+buttonSize) {
				enableCPU = !enableCPU;

				if(!enableCPU) {
					addr = data = 0;
				}
			} else if(x >= resetX && x < resetX+buttonSize && y >= resetY && y < resetY+buttonSize) {
				while(cpuLock == 1) {}
				cpuLock = 1;
				MCS6502Reset(&context);
				cpuLock = 0;
			}

			else if(!enableCPU) {
				for(int i = 0; i < 16; ++i) {
					int bx = addressX + (16*i);
					int by = addressY;
					if(x >= bx && x < bx+buttonSize && y >= by && y < by+buttonSize) {
						int bit = (addr >> (15-i)) & 1;
						bit = !bit;

						addr &= ~(1 << (15-i));
						addr |= bit << (15-i);
					}
				}
				for(int i = 0; i < 8; ++i) {
					int bx = dataX + (16*i);
					int by = dataY;
					if(x >= bx && x < bx+buttonSize && y >= by && y < by+buttonSize) {
						int bit = (data >> (7-i)) & 1;
						bit = !bit;

						data &= ~(1 << (7-i));
						data |= bit << (7-i);
					}
				}

				if(x >= writeX && x < writeX+buttonSize && y >= writeY && y < writeY+buttonSize)
					writeByte(addr, data, NULL);
			}
		}
	}

	CloseWindow();
	running = 0;
	return NULL;
}


int main(int argc, char **argv) {
	MCS6502Init(&context, readByte, writeByte, NULL);

	if(argc > 1) {
		// Read RAM file
		FILE *ramFile = fopen(argv[1], "rb");
		if(!ramFile) {
			perror(argv[1]);
			return 1;
		}
		
		fseek(ramFile, 0, SEEK_END);
		size_t rs = ftell(ramFile);
		fseek(ramFile, 0, SEEK_SET);

		fread(RAM, 1, rs, ramFile);
		fclose(ramFile);
	}

	// Read ROM file
	const char *romPath = "rom.bin";
	FILE *romFile = fopen(romPath, "rb");
	if(!romFile) {
		perror(argv[0]);
		return 1;
	}

	fseek(romFile, 0, SEEK_END);
	size_t s = ftell(romFile);
	fseek(romFile, 0, SEEK_SET);

	fread(ROM, 1, s<ROM_SIZE? s : ROM_SIZE, romFile);
	fclose(romFile);


	// Dispatch render thread
	pthread_t renderThread;

	pthread_create(&renderThread, NULL, render, NULL);

	MCS6502Reset(&context);
	while(running) {
		struct timespec start, end;
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		if(enableCPU && !cpuLock) {
			cpuLock = 1;
			MCS6502Tick(&context);
			cpuLock = 0;
		}
		uint64_t delta = 0;
		while(delta < NANOSECS / CLOCK_FREQ) {
			clock_gettime(CLOCK_MONOTONIC_RAW, &end);
			delta = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);
		}
	}
}
