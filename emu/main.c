#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include <raylib.h>

#include "MCS6502.h"

#define CLOCK_FREQ 1000000
#define MICROSECS  1000000

#define RAM_SIZE 0x8000
#define ROM_SIZE 0x2000
#define RAM_BASE 0x0000
#define ROM_BASE 0xe000

static uint8_t RAM[RAM_SIZE] = {0};
static uint8_t ROM[ROM_SIZE] = {0};
static MCS6502ExecutionContext context;

static int running = 1;
static volatile int cpuLock = 0;
static int enableCPU = 0;

uint8_t readByte(uint16_t addr, void *ctx) {
	if(RAM_BASE <= addr && addr < RAM_BASE+RAM_SIZE) 
		return RAM[addr];

	if(ROM_BASE <= addr && addr < ROM_BASE+ROM_SIZE)
		return ROM[addr & 0b0001111111111111];

	return 0xff;
}


void writeByte(uint16_t addr, uint8_t data, void *ctx) {
	if(RAM_BASE < addr && addr < RAM_BASE+RAM_SIZE)
		RAM[addr] = data;

	if(addr == 0x8000)
		printf("%c", (char)data);
}


void *render(void *args) {
	InitWindow(640, 480, "TSC-8 Emulator");
	SetTargetFPS(60);

	uint16 addr = 0;
	uint8  data = 0;

	const int buttonSize = 25;

	const int enableX = 50;
	const int enableY = 30;

	const int resetX  = 200;
	const int resetY  = 30;

	const int addressX = 30;
	const int addressY = 90;
	const int dataX = 30;
	const int dataY = addressY+50;

	const int writeX = 30;
	const int writeY = dataY + 50;

	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(GRAY);

		DrawText("Enable CPU", 10, 10, 20, WHITE);
		DrawText("Reset CPU", 150, 10, 20, WHITE);
		DrawText("Address", 30, addressY-20, 20, WHITE);
		DrawText("Data", 30, dataY-20, 20, WHITE);
		DrawText("Write", 30, writeY-20, 20, WHITE);

		DrawRectangle(enableX, enableY, buttonSize, buttonSize, enableCPU? GREEN : RED);	
		DrawRectangle(resetX, resetY, buttonSize, buttonSize, RED);	
		DrawRectangle(writeX, writeY, buttonSize, buttonSize, RED);	

		if(enableCPU) {
			addr = context.pc;
			data = readByte(addr, NULL);
		}

		for(int i = 0; i < 16; ++i) {
			int x = addressX + (35*i);
			int y = addressY;

			int bit = (addr >> (15-i)) & 1;
			DrawRectangle(x, y, buttonSize, buttonSize, bit? GREEN : RED);
		}	

		for(int i = 0; i < 8; ++i) {
			int x = dataX + (35*i);
			int y = dataY;

			int bit = (data >> (7-i)) & 1;
			DrawRectangle(x, y, buttonSize, buttonSize, bit? GREEN : RED);
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
					int bx = addressX + (35*i);
					int by = addressY;
					if(x >= bx && x < bx+buttonSize && y >= by && y < by+buttonSize) {
						int bit = (addr >> (15-i)) & 1;
						bit = !bit;

						addr &= ~(1 << (15-i));
						addr |= bit << (15-i);
					}
				}
				for(int i = 0; i < 8; ++i) {
					int bx = dataX + (35*i);
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
			MCS6502ExecNext(&context);
			cpuLock = 0;
		}
		uint64_t delta = 0;
		while(delta < MICROSECS / CLOCK_FREQ) {
			clock_gettime(CLOCK_MONOTONIC_RAW, &end);
			delta = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
		}
	}
}
