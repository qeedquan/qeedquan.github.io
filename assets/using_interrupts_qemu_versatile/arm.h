#define SVC_MODE 0x13
#define NO_INT 0xc0

/*
 * new instructions
 */

#define ISB          \
	MOVW $0, R0; \
	MCR CpSC, 0, R0, C(CpCACHE), C(CpCACHEinvi), CpCACHEwait

#define DSB          \
	MOVW $0, R0; \
	MCR CpSC, 0, R0, C(CpCACHE), C(CpCACHEwb), CpCACHEwait

#define BARRIERS \
	ISB;     \
	DSB

/*
 * Coprocessors
 */
#define CpOFPA 1 /* ancient 7500 FPA */
#define CpFP 10  /* float FP, VFP cfg. */
#define CpDFP 11 /* double FP */
#define CpSC 15  /* System Control */

/*
 * Primary (CRn) CpSC registers.
 */
#define CpID 0      /* ID and cache type */
#define CpCONTROL 1 /* miscellaneous control */
#define CpTTB 2     /* Translation Table Base(s) */
#define CpDAC 3     /* Domain Access Control */
#define CpFSR 5     /* Fault Status */
#define CpFAR 6     /* Fault Address */
#define CpCACHE 7   /* cache/write buffer control */
#define CpTLB 8     /* TLB control */
#define CpCLD 9     /* L2 Cache Lockdown, op1==1 */
#define CpTLD 10    /* TLB Lockdown, with op2 */
#define CpVECS 12   /* vector bases, op1==0, Crm==0, op2s (cortex) */
#define CpPID 13    /* Process ID */
#define CpSPM 15    /* system performance monitor (arm1176) */

/*
 * CpCACHE Secondary (CRm) registers and opcode2 fields.  op1==0.
 * In ARM-speak, 'flush' means invalidate and 'clean' means writeback.
 */
#define CpCACHEintr 0    /* interrupt (op2==4) */
#define CpCACHEisi 1     /* inner-sharable I cache (v7) */
#define CpCACHEpaddr 4   /* 0: phys. addr (cortex) */
#define CpCACHEinvi 5    /* instruction, branch table */
#define CpCACHEinvd 6    /* data or unified */
#define CpCACHEinvu 7    /* unified (not on cortex) */
#define CpCACHEva2pa 8   /* va -> pa translation (cortex) */
#define CpCACHEwb 10     /* writeback */
#define CpCACHEinvdse 11 /* data or unified by mva */
#define CpCACHEwbi 14    /* writeback+invalidate */

#define CpCACHEall 0       /* entire (not for invd nor wb(i) on cortex) */
#define CpCACHEse 1        /* single entry */
#define CpCACHEsi 2        /* set/index (set/way) */
#define CpCACHEtest 3      /* test loop */
#define CpCACHEwait 4      /* wait (prefetch flush on cortex) */
#define CpCACHEdmbarr 5    /* wb only (cortex) */
#define CpCACHEflushbtc 6  /* flush branch-target cache (cortex) */
#define CpCACHEflushbtse 7 /* ⋯ or just one entry in it (cortex) */

/*
 * CpCACHERANGE opcode2 fields for MCRR instruction (armv6)
 */
#define CpCACHERANGEinvi 5  /* invalidate instruction  */
#define CpCACHERANGEinvd 6  /* invalidate data */
#define CpCACHERANGEdwb 12  /* writeback */
#define CpCACHERANGEdwbi 14 /* writeback+invalidate */

/*
 * Program Status Registers
 */
#define PsrMusr 0x00000010 /* mode */
#define PsrMfiq 0x00000011
#define PsrMirq 0x00000012
#define PsrMsvc 0x00000013 /* `protected mode for OS' */
#define PsrMmon 0x00000016 /* `secure monitor' (trustzone hyper) */
#define PsrMabt 0x00000017
#define PsrMund 0x0000001B
#define PsrMsys 0x0000001F /* `privileged user mode for OS' (trustzone) */
#define PsrMask 0x0000001F

#define PsrDfiq 0x00000040 /* disable FIQ interrupts */
#define PsrDirq 0x00000080 /* disable IRQ interrupts */

#define PsrV 0x10000000 /* overflow */
#define PsrC 0x20000000 /* carry/borrow/extend */
#define PsrZ 0x40000000 /* zero */
#define PsrN 0x80000000 /* negative/less than */
