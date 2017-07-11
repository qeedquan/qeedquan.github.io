---
layout: post
title:  "Using Interrupts on the QEMU Versatile board"
date:   2017-07-10
categories: plan9 arm qemu versatile
---

Interrupts are an important component of a computer system. It enables interactivity, priorities, and preemption for
the executing task. Interrupts allowing one to switch out tasks, have periodic timer events, notifications of when
network packets appear, and much more. Most devices on the computer have the ability of generating an interrupt to notify
the processor a state change.

This tutorial will show how to use the interrupts for the [QEMU Versatile]. The code is based on the previous [article],
except we will use interrupts for keyboard/mouse events and enable timer interrupts.

### Main

Initialization stays the same for the most part as the previous [article],
but now we initialize the interrupts with `trapinit` and enable them in `intrson`.

In our `main`, we get rid of `event` handling and instead use interrupt
for keyboard and mouse events. The one shot timer interrupt is also exercised
using the `schedevent` function to schedule a one shot timer interrupt `sc` milliseconds
from now. We will cover these functions more in depth later in the tutorial.

{% highlight c %}
// main.c

void
main(void)
{
	u32 sc;

	// setup UART for printing to terminal
	uartinit();

	// initialize interrupt handlers
	trapinit();

	// setup timer so we can sleep
	timerinit();

	// setup the display so we can draw to the screen
	clcdinit();

	// setup keyboard and mouse
	inputinit();

	// enable interrupts
	intrson();

	// game style loop
	sc = 1000;
	for (;;) {
		if (!scheduled) {
			schedevent(sc);
			sc += 1000;
			if (sc >= 1000 * 1000)
				sc = 1000;
		}
		draw();
	}
}
{% endhighlight %}

### Interrupt handlers

We based our exception handling code from the [Plan9 BCM] port, which uses a CPU that has basically the same features
as the Versatile architecture when dealing with interrupts.

Interrupts are initialized in the `trapinit` function. `trapinit` sets up the exception handler for ARM and the stacks
for them on execution. Since we do not care about any other exceptions, we stub them all out except
for the IRQ exception which the CPU will jump to on an interrupt trigger.

By default, ARM expects the exception handler to be place at address 0x0, but due to the Plan9 linker limitation, we can't
just place the exception table at 0x0, so we define the exception handler tables in `lexception.s` called `vectors`
that sets the PC to the exception handler address defined in `vtable`.

Afterwards, we set up the stacks for the interrupts with `setr13`. When an exception occurs, ARM will switch mode and use a different set of banked registers, which includes the stack pointer. We only give enough stack space to save the registers
R0-R4 because we intend to use them as scratch register to switch to supervisor (svc) mode. Afterwards, we will be using
the stack we setup in `l.s` before we called `main`.

Then we need to enable the secondary interrupt controller. The Versatile has two interrupt controllers, a primary
one and a secondary one that sends its interrupt to the primary one, but to do that, we would need to enable it first.
The keyboard and mouse interrupts are in the secondary interrupt controller.

After we setup everything, interrupts will occur inside `_virq` handler inside `lexception.s`, which will call `trap` function to handle the interrupt in C. We enable the interrupts we want to handle by writing to the [PL190 Interrupt Controller]
register to enable them. The function `intrenable` connects a function handler for an interrupt line. The interrupt line
is defined in the [Versatile Datasheet].

For a more detailed information about the ARM exception handling and interrupt lines, refer to the following docs:
[Versatile Datasheet], [ARM Exception Tutorial], and [PL190 Interrupt Controller].

{% highlight assembly %}
// lexception.s

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
	// we don't nest interrupts
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

{% endhighlight %}

{% highlight c %}
// trap.c

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

{% endhighlight %}

### Timers, keyboard, mouse interrupts
We have setup the interrupt handling system, now we just need to tell the devices to enable the interrupts
and connect them. For timer, we hook one timer up as a periodic timer, triggering every once a second, and then
we use another timer as a one-shot timer, usable by `schedevent`. We do not need to worry about locking here
since we do not nesting interrupts and we run on a single core. For keyboard and mouse, we need to enable the RX 
interrupt. In the interrupt handler for these devices, we need to clear the interrupt register or else
they will keep the line asserted, generating an interrupt storm.

{% highlight c %}
// timer.c

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
{% endhighlight %}

{% highlight c %}
// input.c
#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

// registers, 32 bit wide
enum {
	CTRL = 0,
	STAT,
	DATA,
	CLKDIV,
	IR,
};

// control register bits
enum {
	ENABLE = 1 << 2,
	INTTX = 1 << 3,
	INTRX = 1 << 4,
};

// status register bits
enum {
	RXFULL = 0x10,
};

static Input physinput[] = {
    {
        .r = (void *)0x10006000,
    },
    {
        .r = (void *)0x10007000,
        .ismouse = true,
    },
};

// send data and drain input
static void
send(Input *p, u32 v)
{
	p->r[DATA] = v;
	while (p->r[STAT] & RXFULL)
		v = p->r[DATA];
}

// reset the device
static void
reset(Input *p)
{
	// enable and get rx interrupts
	p->r[CTRL] = ENABLE | INTRX;

	// this enable the ps2 interface for usage
	// without this we won't get the mouse events
	if (p->ismouse)
		send(p, 0xf4);
}

// initializes the input
void
inputinit(void)
{
	reset(&physinput[0]);
	reset(&physinput[1]);
}

// polls for input if there is no input
// set events to 0
void
pollinput(u32 *kb, u32 *ms)
{
	Input *p;
	int i;

	*kb = *ms = 0;
	for (i = 0; i < nelem(physinput); i++) {
		p = &physinput[i];
		if (!(p->r[STAT] & RXFULL))
			continue;

		if (i == 0) {
			// keyboard just gets raw data
			// ignore 0xf0, that is a "end marker"
			*kb = p->r[DATA];
			if (*kb == 0xf0)
				*kb = 0;
		} else {
			// mouse gets 3 packet, the
			// status, dx, dy
			*ms |= p->r[DATA];
			*ms |= (p->r[DATA] << 8);
			*ms |= (p->r[DATA] << 16);
		}
	}
}

// move the cursor based on the mouse movement
void
updatecursor(Cursor *c, u32 ms)
{
	// move x and y, the mouse events
	// switches the y coordinates around
	c->dx = (s8)((ms >> 8) & 0xff);
	c->dy = -(s8)((ms >> 16) & 0xff);
	c->x += c->dx;
	c->y += c->dy;

	// bounds checking so the cursor does
	// not get out of the screen
	if (c->x < 0)
		c->x = 0;
	if (c->x + c->w >= screen->w)
		c->x = screen->w - c->w;

	if (c->y < 0)
		c->y = 0;
	if (c->y + c->h >= screen->h)
		c->y = screen->h - c->h;
}

// input interrupt
void
inputinr(Ureg *, void *)
{
	// we handle events in the interrupt handler
	event();
}

{% endhighlight %}

### Conclusion
Here is a sample output from what you should see if all went well:

The timer #1 interrupt is periodic and generates one time a second.

The timer #2 interrupt is one-shot and generated one time a second, with delays increasing every 1000 milliseconds
every time through the loop.

The keyboard and mouse interrupt is generated when the user uses the device.

{% highlight none %}
timer #1 periodic interrupt
timer #2 one shot interrupt
timer #1 periodic interrupt
timer #1 periodic interrupt
timer #2 one shot interrupt
timer #1 periodic interrupt
timer #1 periodic interrupt
timer #1 periodic interrupt
timer #2 one shot interrupt
timer #1 periodic interrupt
timer #1 periodic interrupt
key 1c
key 1c
key 2b
key 2b
timer #1 periodic interrupt
key 25
key 42
key 25
key 42
timer #1 periodic interrupt
key 14
key 11
timer #2 one shot interrupt
key 14
key 11
key 11
{% endhighlight %}

You can get the code for the demo [here].

[ARM Exception Tutorial]: http://osnet.cs.nchu.edu.tw/powpoint/Embedded94_1/Chapter%207%20ARM%20Exceptions.pdf
[article]: https://qeedquan.github.io/plan9/arm/qemu/versatile/2017/06/21/Using-QEMU-Versatile-Display-Input.html
[here]: https://github.com/qeedquan/qeedquan.github.io/blob/master/assets/using_interrupts_qemu_versatile
[PL190 Interrupt Controller]: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0181e/DDI0181.pdf
[Plan9 BCM]: https://code.9front.org/hg/plan9front/file/d44f7b86e2ba/sys/src/9/bcm
[QEMU Versatile]: https://github.com/qemu/qemu/blob/master/hw/arm/versatilepb.c
[Versatile Datasheet]: http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
