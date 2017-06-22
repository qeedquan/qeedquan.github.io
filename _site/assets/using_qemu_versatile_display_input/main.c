#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

// cursor on the screen
Cursor cursor = {
    .w = 15,
    .h = 15,
};

// our screen
Clcd *screen;

// the artwork we are drawing
Texture pic;

// setup the texture attribute
// for rendering
static void
artinit(void)
{
	extern u32 art[];
	Texture *t;

	t = &pic;
	t->p = art;
	t->w = 190;
	t->h = 222;
}

// get keyboard and mouse events
static void
event(void)
{
	u32 kb, ms;

	for (;;) {
		pollinput(&kb, &ms);
		if (kb == 0 && ms == 0)
			break;

		updatecursor(&cursor, ms);

		if (kb)
			print("key %x\n", kb);
	}
}

static void
draw(void)
{
	Rect r;

	// disable the screen so we can draw to it
	// if we enable the screen while drawing, it can cause
	// the update to show partial updates, where as we want
	// something like double buffering, this achieves something
	// like double-buffering
	clcddisable(screen);

	// clear to a gray background
	fillrect(0, 0, screen->w, screen->h, 0xff555555);

	// draw a image at the center of the screen
	r = (Rect){(screen->w - pic.w) / 2, (screen->h - pic.h) / 2, pic.w, pic.h};
	filltexture(&pic, &r, nil);

	// draw our cursor
	fillrect(cursor.x, cursor.y, cursor.w, cursor.h, 0xff00ff00);

	// draw the device, we need to delay a little so QEMU can have a chance to refresh
	// if we do not have a delay, QEMU can get into a execution path where it only
	// sees the disable and not the enable when it decides to refresh, thus giving
	// us a black screen.
	clcdenable(screen);
	delay(5);
}

void
main(void)
{
	// setup UART for printing to terminal
	uartinit();

	// setup timer so we can sleep
	timerinit();

	// setup the display so we can draw to the screen
	clcdinit();

	// setup keyboard and mouse
	inputinit();

	// setup texture for drawing
	artinit();

	// game style loop
	for (;;) {
		event();
		draw();
	}
}
