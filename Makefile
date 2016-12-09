TOOLCHAIN=arm-none-eabi-
LINKSCRIPT=link.ld
LDFLAGS=-T $(LINKSCRIPT)
CFLAGS=-c -mcpu=cortex-m3 -mthumb -g -I. -fshort-wchar
ASFLAGS=-c -mcpu=cortex-m3 -mthumb -g -I.
CC=$(TOOLCHAIN)gcc
LD=$(TOOLCHAIN)ld
AS=$(TOOLCHAIN)gcc -x assembler-with-cpp
OBJCOPY=$(TOOLCHAIN)objcopy

ASSOURCES=$(wildcard *.S)
SOURCES=$(wildcard *.c)
ASOBJECTS=$(SOURCES:.S=.o)
OBJECTS=$(SOURCES:.c=.o)
BINARY=out.bin
HEX=out.hex

SERIAL=/dev/ttyUSB1
BAUD=115200

$(HEX): $(BINARY) reset
	$(OBJCOPY) out.bin -O ihex out.hex

$(BINARY): $(OBJECTS) $(ASOBJECTS) startup.o
	$(LD) $(LDFLAGS) $(OBJECTS) startup.o -o out.bin

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm *.o *.bin *.hex reset

reset: reset_tool/reset.c
	gcc reset_tool/reset.c -o reset

run:
	sudo minicom -D $(SERIAL) -b $(BAUD)

program: out.hex
	sudo lpc21isp -control out.hex $(SERIAL) $(BAUD) 14746
