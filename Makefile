all: rom.bin tsc8

ASM = ca65
ALD = ld65

CC = gcc
LD = gcc

CFLAGS = -g -c -Wall
LDFLAGS = -lraylib

ASMFLAGS = 
ALDFLAGS = -Ctsc8-ld65.conf

SRC = $(wildcard rom/*.s)
OBJ = obj/main.o
BIN = rom.bin

EMUSRC = $(wildcard emu/*.c)
EMUHDR = $(wildcard emu/*h)
EMUOBJ = $(EMUSRC:emu/%.c=obj/e_%.o)
EMUBIN = tsc8

$(EMUBIN): $(EMUOBJ)
	$(LD) $(LDFLAGS) -o $@ $^

obj/e_%.o: emu/%.c $(EMUHDR)
	$(CC) $(CFLAGS) -o $@ $<


$(BIN): $(OBJ) $(SRC)
	$(ALD) $(ALDFLAGS) -o $@ $<

obj/%.o: rom/%.s $(SRC)
	$(ASM) $(ASMFLAGS) -o $@ $<
