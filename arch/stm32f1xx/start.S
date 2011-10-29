
.syntax unified

.globl _start

#define VECTOR(name) \
	.long handle_##name + 1; \
	.weak handle_##name ; \
	.thumb_set handle_##name, deadloop

#define _IRQ(name) VECTOR(irq_##name)

_start:
	.long 0x20005000
	.long reset + 1
	VECTOR(m3_nmi)
	VECTOR(m3_hardfault)
	VECTOR(m3_mmu)
	VECTOR(m3_busfault)
	VECTOR(m3_usagefault)
	VECTOR(m3_reserved_a)
	VECTOR(m3_reserved_b)
	VECTOR(m3_reserved_c)
	VECTOR(m3_reserved_d)
	VECTOR(m3_svc)
	VECTOR(m3_debugmon)
	VECTOR(m3_reserved_e)
	VECTOR(m3_pendsv)
	VECTOR(m3_systick)

#include <arch/irqs.h>

deadloop:
	b .

reset:
	ldr r1, =__data_init__
	ldr r2, =__data_start__
	ldr r3, =__data_end__
	ldr r4, =__bss_end__
	mov r5, #0

	/* if data init and start are the same, skip copy */
	/* this simplifies building-for-ram */
	cmp r1, r2
	bne copydata
	mov r2, r4
	b zerobss

copydata:
	cmp r2, r3
	beq zerobss
	ldr r0, [r1], #4
	str r0, [r2], #4
	b copydata
zerobss:
	cmp r2, r4
	beq main
	str r5, [r2], #4
	b zerobss
