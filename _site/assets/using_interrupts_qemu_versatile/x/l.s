#include "arm.h"

// start execution here
TEXT _start(SB), 1, $-4
	// supervisor mode and no interrupt 
	MOVW $(SVC_MODE|NO_INT), R1
	MOVW R1, CPSR

	// setup R12 for global variable access
	MOVW $setR12(SB), R12

	// setup stack and jump to C
	MOVW $0x8000, SP
	BL main(SB)

	// loop forever
	B 0(PC)

/*
 * drain write buffer and prefetch buffer
 * writeback and invalidate data cache
 * invalidate instruction cache
 */
TEXT cacheuwbinv(SB), 1, $-4
	BARRIERS
	MOVW	$0, R0
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEwbi), CpCACHEall
	MCR	CpSC, 0, R0, C(CpCACHE), C(CpCACHEinvi), CpCACHEall
	RET

TEXT coherence(SB), $-4
	BARRIERS
	RET

TEXT spllo(SB), 1, $-4
	MOVW CPSR, R0			/* turn on irqs and fiqs */
	BIC	$(PsrDirq|PsrDfiq), R0, R1
	MOVW R1, CPSR
	RET

TEXT splhi(SB), 1, $-4
	MOVW CPSR, R0			/* turn off irqs (but not fiqs) */
	ORR	$(PsrDirq), R0, R1
	MOVW R1, CPSR
	RET

TEXT splx(SB), 1, $-4
	MOVW	R0, R1				/* reset interrupt level */
	MOVW	CPSR, R0
	MOVW	R1, CPSR
	RET
