#define SVC_MODE 0x13
#define NO_INT 0xc0

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

