ASM = ca65
LD  = ld65

ASMFLAGS = 
LDFLAGS = -Ctsc8-ld65.conf

SRC = rom/main.s
OBJ = obj/main.o
BIN = rom.bin

$(BIN): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

obj/%.o: rom/%.s
	$(ASM) $(ASMFLAGS) -o $@ $<
