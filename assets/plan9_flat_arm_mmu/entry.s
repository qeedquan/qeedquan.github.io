#include "memlayout.h"
#include "arm.h"
#include "mmu.h"

TEXT _start(SB), 1, $-4
	// clear the memory for data structures we need
	MOVW	$(edata_entry), R1
	MOVW	$(end_entry), R2
	MOVW	$0, R3
_zero:
	MOVW	R3, (R1)
	ADD	$4, R1
	CMP	R1, R2
	BNE	_zero

	// set supervisor mode, no interrupts
	MOVW	$(SVC_MODE|NO_INT), R1
	MOVW	R1, CPSR

	// set the stack pointer to jump into C
	MOVW	$(svc_stktop), SP
	BL	start(SB)

	// loop forever
	B	0(PC)

// loads the page tables for kernel and user
// void load_pgtbl(u32 *kernel_pgtbl, u32 *user_pgtbl)
// R0    - kernel_pgtbl
// SP[8] - user_pgtbl
TEXT load_pgtbl(SB), 1, $-4
	// set the domain access control; all domains are checked for permission
	MOVW	$0x55555555, R3
	MCR	15, 0, R3, C(3), C(0), 0

	// set the page table base registers; we use two tables:
	// TTBR0 for user space and TTBR1 for kernel space
	MOVW	$(32-UADDR_BITS), R3
	MCR	15, 0, R3, C(2), C(0), 2

	// load the kernel page table
	MOVW	R0, R3
	MCR	15, 0, R3, C(2), C(0), 1

	// load the user page table
	MOVW	8(SP), R3
	MCR	15, 0, R3, C(2), C(0), 0

	// enable MMU, cache, write buffer, high vector tbl,
	// disable subpage
	MRC	15, 0, R3, C(1), C(0), 0
	ORR	$0x80300D, R3
	MCR	15, 0, R3, C(1), C(0), 0

	// flush the TLB
	MOVW	$0, R3
	MCR	15, 0, R3, C(8), C(7), 0

	RET

// once we get here, we have enabled the MMU and setup the page tables
// when the kernel booted up, it was in user address space, but
// now we can use the kernel address space now for the kernel
TEXT jump_stack(SB), 1, $-4
	// R12 defined by the linker is relative to the kernel address
	// so we couldn't use it until now
	MOVW	$setR12(SB), R12
	
	// setup stack pointer to be in kernel virtual address now
	ADD	$(KERNBASE), SP

	// jump to the address of main, main is in the kernel address range
	// we couldn't load this before the MMU is setup properly
	MOVW	$kmain(SB), PC

