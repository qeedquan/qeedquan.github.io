#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

// timer registers, 32 bit wide
enum {
	LOAD = 0x0,
	VALUE,
	CTRL,
	INTCLR,
	RIS,
	MIS,
};

// control bits
enum {
	ENABLE = 0x80,
	PERIODIC = 0x40,
	INTENABLE = 0x20,
	RESLN32 = 0x2,
	ONESHOT = 0x1,
};

enum {
	CLOCKFREQ = 1 * MHZ,
};

Timer phystimer[4] = {
    {
        .r = (void *)0x101e2000,
    },
    {
        .r = (void *)(0x101e2000 + 0x20),
    },
    {
        .r = (void *)0x101e3000,
    },
    {
        .r = (void *)(0x101e3000 + 0x20),
    },
};

// reset the timer
static void
reset(Timer *t, bool periodic, bool oneshot, bool irq)
{
	u32 v;

	// if periodic, count at MHZ rate
	// 1 interrupt per second
	if (periodic)
		t->r[LOAD] = CLOCKFREQ;

	// enable the device with 32 bit counter wide
	v = ENABLE | RESLN32;

	// enable irq if we use it
	if (irq)
		v |= INTENABLE;

	// enable periodic if we use it
	if (periodic)
		v |= PERIODIC;

	if (oneshot) {
		v |= ONESHOT;
		v &= ~ENABLE;
	}

	t->r[CTRL] = v;
}

// initializes the timer
void
timerinit(void)
{
	reset(&phystimer[0], false, false, false);
	reset(&phystimer[1], true, false, true);
	reset(&phystimer[2], false, true, true);
}

// delay for n milliseconds
void
delay(int n)
{
	while (--n >= 0)
		microdelay(1000);
}

// delay for n microseconds
void
microdelay(int n)
{
	Timer *t;
	u32 a, b;

	// we do not need to worry about underflow here
	// since the unsignedness of the values ensure
	// that the wrap around calculation is still right
	t = &phystimer[0];
	a = t->r[VALUE];
	while (a - t->r[VALUE] < n + 1)
		;
}

// handle periodic timer interrupt
void
timerintr(Ureg *, void *)
{
	Timer *t;

	t = &phystimer[1];
	t->r[INTCLR] = 1;
	iprint("timer #1 periodic interrupt\n");
}

int scheduled;

void
timeroneintr(Ureg *, void *)
{
	Timer *t;
	t = &phystimer[2];
	t->r[INTCLR] = 1;
	iprint("timer #2 one shot interrupt\n");
	scheduled--;
}

// schedule event in number of ms from now
void
schedevent(u32 ms)
{
	Timer *t;

	if (scheduled)
		return;

	scheduled++;

	t = &phystimer[2];
	t->r[LOAD] = ms * 1000;
	t->r[CTRL] |= ENABLE;
}