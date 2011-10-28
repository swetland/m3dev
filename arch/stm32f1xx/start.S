
.syntax unified

.globl _start

#define VECTOR(name) \
	.long handle_##name + 1; \
	.weak handle_##name ; \
	.thumb_set handle_##name, deadloop

#define _IRQ(name) VECTOR(irq_##name)

_start:
	//.long 0x200001fc
	//.long 0x1ffff021
	.long 0x20005000
	.long m3_reset - 0x20001000 + 0x08000000 + 1
	//VECTOR(m3_reset)
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

m3_reset:
	ldr r1, =0x08000000
	ldr r2, =0x20001000
	ldr r3, =0x20004000
copymore:
	ldr r0, [r1], #4
	str r0, [r2], #4
	cmp r2, r3
	bne copymore
	ldr r1, =main
	bx r1
