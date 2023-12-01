#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

// base physical address
// of the CLCD device
enum {
	CLCD = 0x10120000,
};

// the registers offset
// each is 32 bit wide
enum {
	TIM0 = 0x0,
	TIM1 = 0x1,
	TIM2 = 0x2,
	TIM3 = 0x3,
	UPBASE = 0x4,
	LPBASE = 0x5,
	CTRL = 0x6,
	IMSC = 0x7,
	ICR = 0xa,
	SYS = 0x14,
	PAL = 0x80,
};

// system register bits
enum {
	PWR3V5 = 0x10,
	LCDIOON = 0x4,
};

// control register bits
enum {
	CR_EN = 0x1,
	CR_BGR = 0x100,
	CR_BEBO = 0x200,
	CR_BEPO = 0x400,
	CR_PWR = 0x800,
};

// color depth
enum {
	BPP1 = 0,
	BPP2,
	BPP4,
	BPP8,
	BPP16,
	BPP32,
	BPP16_565,
	BPP12,
};

Clcd physclcd[] = {
    {
        .r = (void *)CLCD,
        .w = 640,
        .h = 480,
        .fb = (void *)0x150000,
    },
};

// resets the CLCD controller
static void
reset(Clcd *c)
{
	// turn on the device
	c->r[SYS] = PWR3V5 | LCDIOON;

	// setup width/height
	c->r[TIM0] = c->w / 4 - 4;
	c->r[TIM1] = c->h - 1;

	// frame buffer location
	c->r[UPBASE] = (uintptr)c->fb;

	// 32 bit color (rgba)
	c->r[CTRL] = (BPP32 << 1);
}

// disable the screen, it leaves whatever was on
// the screen intact
void
clcddisable(Clcd *c)
{
	c->r[CTRL] &= ~(CR_PWR | CR_EN);
}

// enable the screen, QEMU will start updating
// the frame buffer again
void
clcdenable(Clcd *c)
{
	c->r[CTRL] |= (CR_PWR | CR_EN);
}

// initializes the CLCD display controller
void
clcdinit(void)
{
	reset(&physclcd[0]);
	screen = &physclcd[0];
	fillrect(0, 0, screen->w, screen->h, 0xff555555);
	microdelay(5);
}
