#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

enum {
	SICREGS = 0x10003000,
	INTREGS = 0x10140000,
};

enum {
	SICSTAT = 0,
	SICRAWSTAT = 1,
	SICENABLE = 2,
	SICSOFTINT = 3,
	SICPICENABLE = 8,

	SICENSET = 2,
	SICENCLR = 3,
	SICSOFTINTSET = 4,
	SICSOFTINCLR = 5,
	SICPICENSET = 8,
	SICPICENCLR = 9,
};

enum {
	INTSTAT = 0,
	FIQSTAT,
	INTRAW,
	INTSELECT,
	INTENABLE,
	INTCLEAR,
	SOFTINT,
	SOFTINTCLEAR,
	PROTECTION,
	VECTADDR,
};

enum {
	// # of vectors at start of lexception.s
	NVEC = 8,

	// # number of interrupt lines
	NINTR = 32,
};

// interrupt vectors
static Vctl vctls[NINTR];

// save areas for exceptions, hold R0-R4
static u32 sfiq[4096];
static u32 sirq[5];
static u32 sund[5];
static u32 sabt[5];
static u32 smon[5];
static u32 ssys[5];

// layout of vector table
typedef struct Vpage0 {
	void (*vectors[NVEC])(void);
	u32 vtable[NVEC];
} Vpage0;

// initializes interrupt handling
void
trapinit(void)
{
	u32 *sic;
	Vpage0 *vpage0;

	// turn off interrupts
	intrsoff();

	// the exception table is stored at address 0
	vpage0 = (void *)0;
	memmove(vpage0->vectors, vectors, sizeof(vpage0->vectors));
	memmove(vpage0->vtable, vtable, sizeof(vpage0->vtable));
	cacheuwbinv();

	// set up the stacks for the interrupt modes
	setr13(PsrMfiq, sfiq);
	setr13(PsrMirq, sirq);
	setr13(PsrMabt, sabt);
	setr13(PsrMund, sund);
	setr13(PsrMsys, ssys);
	coherence();

	// enable secondary interrupts
	sic = (void *)SICREGS;
	sic[SICENSET] = 0xffffffff;
	sic[SICPICENSET] = 0xffffffff;
	coherence();
}

// turn interrupts on
void
intrson(void)
{
	intrenable(TIMER0IRQ, timerintr, nil, "timer0");
	intrenable(TIMER2IRQ, timeroneintr, nil, "timer2");
	intrenable(VIC31IRQ, inputinr, nil, "input");

	spllo();
}

// turn interrupts off
void
intrsoff(void)
{
	u32 *ip;

	ip = (void *)INTREGS;
	ip[INTCLEAR] = 0;
	coherence();
}

// enable an interrupt line
void
intrenable(int irq, void (*f)(Ureg *, void *), void *arg, char *name)
{
	u32 *ip;
	Vctl *v;

	if (irq < 0 || irq >= NINTR)
		panic("invalid irq %d", irq);

	v = &vctls[irq];
	v->f = f;
	v->a = arg;
	v->name = name;

	ip = (void *)INTREGS;
	ip[INTENABLE] |= (1 << irq);
	coherence();
}

// handle interrupts
static void
irq(Ureg *ureg)
{
	Vctl *v;
	u32 *ip, i;

	ip = (void *)INTREGS;
	for (i = 0; i < NINTR; i++) {
		if (ip[INTSTAT] & (1 << i)) {
			v = &vctls[i];
			v->f(ureg, v->a);
		}
	}
}

void
trap(Ureg *ureg)
{
	// all interrupts/exceptions should be resumed at ureg->pc-4,
	// except for Data Abort which resumes at ureg->pc-8.
	if (ureg->type == (PsrMabt + 1))
		ureg->pc -= 8;
	else
		ureg->pc -= 4;

	switch (ureg->type) {
	case PsrMirq:
		irq(ureg);
		break;
	default:
		panic("unknown trap: type %x, psr mode %x", ureg->type, ureg->psr & PsrMask);
	}
}
