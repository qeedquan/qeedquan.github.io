#include "arm.h"

// exception table containing branches to code
// that contain the exception handler code. 
// The offset is 0x18 because ARM stores the PC 2 instructions (8 bytes)
// ahead when it is accessed this way.  (24 + 8) = 32 bytes
// The offset points to the vtable structure.
TEXT vectors(SB), 1, $-4
	MOVW 0x18(R15), R15
	MOVW 0x18(R15), R15
	MOVW 0x18(R15), R15
	MOVW 0x18(R15), R15
	MOVW 0x18(R15), R15
	MOVW 0x18(R15), R15
	MOVW 0x18(R15), R15

// table that contains the addreses
// for the exception handler 
TEXT vtable(SB), 1, $-4
	WORD $_vrst(SB)  // reset
	WORD $_vund(SB)  // undefined
	WORD $_vsvc(SB)  // swi
	WORD $_vpabt(SB) // prefetch abort
	WORD $_vdabt(SB) // data abort
	WORD $_vsvc(SB)  // reserved
	WORD $_virq(SB)  // irq
	WORD $_vfiq(SB)  // fiq

// reset vector
TEXT _vrst(SB), 1, $-4
	B 0(PC)
	RFE

// undefined vector
TEXT _vund(SB), 1, $-4
	MOVM.IA	[R0-R4], (R13)
	MOVW $PsrMund, R0
	B _vswitch

// swi interrupt vector
TEXT _vsvc(SB), 1, $-4
	B 0(PC)
	RFE

// prefetch abort vector
TEXT _vpabt(SB), 1, $-4
	MOVM.IA	[R0-R4], (R13)
	MOVW $PsrMabt, R0
	B _vswitch

// data abort vector
TEXT _vdabt(SB), 1, $-4
	MOVM.IA	[R0-R4], (R13)
	MOVW $(PsrMabt+1), R0
	B _vswitch

// irq vector
TEXT _virq(SB), 1, $-4
	MOVM.IA	[R0-R4], (R13)
	MOVW $PsrMirq, R0		/* r0 = type */
	B _vswitch

_vswitch:
	// when we get here, the stack we are using
	// is from setr13, which we only allocate enough
	// just to store the registers, we will use the
	// svc mode stack for all the heavy processing
	
	// save SPSR for ureg
	MOVW SPSR, R1
	// save interrupted PC for ureg
	MOVW R14, R2
	// save pointer to where [R0-R4] are
	MOVW R13, R3
	
	// disable interrupts at this point
	// and switch to svc mode
	// in this context, we are already in svc
	// mode already though
	MOVW CPSR, R14
	BIC	$PsrMask, R14
	ORR	$(PsrDirq|PsrMsvc), R14
	MOVW R14, CPSR
	
	// when we get here, we are no longer using the
	// stack that was defined using setr13, but using
	// the svc stack, which was setup in l.s before we
	// call main
	
	// set ureg->{type, psr, pc}; r13 points to ureg->type
	MOVM.DB.W [R0-R2], (R13)

	// restore [R0-R4] from previous mode's stack
	MOVM.IA (R3), [R0-R4]
	
	// in order to get a predictable value in R13 after the stores,
	// separate the store-multiple from the stack-pointer adjustment
	// we'll assume that the old value of R13 should be stored on the stack
	
	// save kernel level registers, at end r13 points to ureg
	MOVM.DB	[R0-R14], (R13)
	// SP now points to saved R0
	SUB	$(15*4), R13
	
	// first arg is pointer to ureg
	MOVW R13, R0

	BL trap(SB)
	
	// make r13 point to ureg->type
	ADD	$(4*15), R13
	
	// restore link
	MOVW 8(R13), R14

	// restore SPSR
	MOVW 4(R13), R0
	MOVW R0, SPSR
	
	// restore register
	MOVM.DB (R13), [R0-R14]
	// pop past ureg->{type+psr} to pc
	ADD	$(4*2), R13	
	
	// return from exception
	RFE

// fiq vector
TEXT _vfiq(SB), 1, $-4
	B 0(PC)
	RFE

// set the stack value for the mode passed in R0
TEXT setr13(SB), 1, $-4
	// R1 = stack address that we are going to set for
	// the exception handler
	MOVW 4(FP), R1

	// save the current execution mode in R2
	// then switch to new mode by clearing out
	// the mode and ORing it with the mode we want
	// R2 = execution mode
	MOVW CPSR, R2
	BIC	$PsrMask, R2, R3
	ORR	R0, R3
	// switch to new mode
	MOVW R3, CPSR

	// return old sp
	MOVW R13, R0
	// install new sp
	MOVW R1, R13

	// switch back to old mode
	MOVW R2, CPSR
	RET
