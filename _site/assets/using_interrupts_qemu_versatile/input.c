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
	event();
}
