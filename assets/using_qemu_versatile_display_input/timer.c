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
reset(Timer *t)
{
	// enable the device with 32 bit counter wide
	t->r[CTRL] = ENABLE | RESLN32;
}

// initializes the timer
void
timerinit(void)
{
	reset(&phystimer[0]);
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
