#include "hdr/hdr_special_registers.h"

.text
.balign 2
.syntax unified
.thumb
.thumb_func
.global reset_handler

reset_handler:
	ldr r0, =__process_stack_end
	msr PSP, r0

	movs r0, #CONTROL_ALTERNATE_STACK
	msr CONTROL, r0
	isb

	ldr r1, =__data_init_start
	ldr r2, =__data_start
	ldr r3, =__data_end

1:	cmp r2, r3
	ittt lo
	ldrlo r0, [r1], #4
	strlo r0, [r2], #4
	blo 1b

	movs r0, #0
	ldr r1, =__bss_start
	ldr r2, =__bss_end

1:	cmp r1, r2
	itt lo
	strlo r0, [r1], #4
	blo 1b

	ldr		r0, =main
	blx		r0

	b		.

