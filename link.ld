SEARCH_DIR(.);

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm");
OUTPUT_ARCH(arm);

__main_stack_size = 0;
__process_stack_size = 1024;

PROVIDE(__main_stack_size = __main_stack_size);
PROVIDE(__process_stack_size = __process_stack_size);

MEMORY
{
	rom (rx)		: org = 0x00000000, len = 512k
	ram (rwx)		: org = 0x10000000, len = 32k
	ahb_sram0 (rwx)	: org = 0x2007C000, len = 16k
	ahb_sram1 (rwx)	: org = 0x20080000, len = 16k
}

__rom_start = ORIGIN(rom);
__rom_size = LENGTH(rom);
__rom_end = __rom_start + __rom_size;

__ram_start = ORIGIN(ram);
__ram_size = LENGTH(ram);
__ram_end = __ram_start + __ram_size;

__ahb_sram0_start = ORIGIN(ahb_sram0);
__ahb_sram0_size = LENGTH(ahb_sram0);
__ahb_sram0_end = __ahb_sram0_start + __ahb_sram0_size;

__ahb_sram1_start = ORIGIN(ahb_sram1);
__ahb_sram1_size = LENGTH(ahb_sram1);
__ahb_sram1_end = __ahb_sram1_start + __ahb_sram1_size;

PROVIDE(__rom_start = __rom_start);
PROVIDE(__rom_size = __rom_size);
PROVIDE(__rom_end = __rom_end);

PROVIDE(__ram_start = __ram_start);
PROVIDE(__ram_size = __ram_size);
PROVIDE(__ram_end = __ram_end);

PROVIDE(__ahb_sram0_start = __ahb_sram0_start);
PROVIDE(__ahb_sram0_size = __ahb_sram0_size);
PROVIDE(__ahb_sram0_end = __ahb_sram0_end);

PROVIDE(__ahb_sram1_start = __ahb_sram1_start);
PROVIDE(__ahb_sram1_size = __ahb_sram1_size);
PROVIDE(__ahb_sram1_end = __ahb_sram1_end);

ENTRY(reset_handler);

SECTIONS
{
	.text :
	{
		. = ALIGN(4);
		__text_start = .;
		PROVIDE(__text_start = __text_start);

		. = ALIGN(4);
		KEEP(*(.vectors));
		. = ALIGN(4);
		*(.text .text.* .gnu.linkonce.t.*);
		. = ALIGN(4);
		*(.glue_7t .glue_7);
		. = ALIGN(4);
		*(.rodata .rodata.* .gnu.linkonce.r.*);

		. = ALIGN(4);
		__text_end = .;
		PROVIDE(__text_end = __text_end);
	} > rom AT > rom

	.data :
	{
		. = ALIGN(4);
		__data_init_start = LOADADDR (.data);
		PROVIDE(__data_init_start = __data_init_start);
		__data_start = .;
		PROVIDE(__data_start = __data_start);

		. = ALIGN(4);
		*(.data .data.* .gnu.linkonce.d.*)

		. = ALIGN(4);
		__data_end = .;
		PROVIDE(__data_end = __data_end);
	} > ram AT > rom

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		PROVIDE(__bss_start = __bss_start);

		. = ALIGN(4);
		*(.bss .bss.* .gnu.linkonce.b.*)
		. = ALIGN(4);
		*(COMMON);

		. = ALIGN(4);
		__bss_end = .;
		PROVIDE(__bss_end = __bss_end);
	} > ram AT > ram

	.stack :
	{
		. = ALIGN(8);
		__stack_start = .;
		PROVIDE(__stack_start = __stack_start);

		. = ALIGN(8);
		__main_stack_start = .;
		PROVIDE(__main_stack_start = __main_stack_start);

		. += __main_stack_size;

		. = ALIGN(8);
		__main_stack_end = .;
		PROVIDE(__main_stack_end = __main_stack_end);

		. = ALIGN(8);
		__process_stack_start = .;
		PROVIDE(__process_stack_start = __process_stack_start);

		. += __process_stack_size;

		. = ALIGN(8);
		__process_stack_end = .;
		PROVIDE(__process_stack_end = __process_stack_end);

		. = ALIGN(8);
		__stack_end = .;
		PROVIDE(__stack_end = __stack_end);
	} > ram AT > ram

	. = ALIGN(4);
	__heap_start = .;
	PROVIDE(__heap_start = __heap_start);

	. = ALIGN(4);
	__heap_end = __ram_end;
	PROVIDE(__heap_end = __heap_end);

	.stab 				0 (NOLOAD) : { *(.stab) }
	.stabstr 			0 (NOLOAD) : { *(.stabstr) }
}

PROVIDE(__text_size = __text_end - __text_start);
PROVIDE(__exidx_size = __exidx_end - __exidx_start);
PROVIDE(__data_size = __data_end - __data_start);
PROVIDE(__bss_size = __bss_end - __bss_start);
PROVIDE(__stack_size = __stack_end - __stack_start);
PROVIDE(__heap_size = __heap_end - __heap_start);

