TOOLCHAIN=arm-none-eabi-
LINKSCRIPT=test.ld
LDFLAGS=-T $(LINKSCRIPT)
CFLAGS=-c -mcpu=cortex-m3 -mthumb -g -I.
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

$(HEX): $(BINARY)
	$(OBJCOPY) out.bin -O ihex out.hex

$(BINARY): $(OBJECTS) $(ASOBJECTS) startup.o
	$(LD) $(LDFLAGS) $(OBJECTS) startup.o -o out.bin

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm *.o *.bin *.hex

program: out.hex
	sudo lpc21isp out.hex /dev/ttyUSB0 115200 120000000
