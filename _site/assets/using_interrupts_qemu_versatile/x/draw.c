#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

// sets a pixel on the screen
void
setpixel(int x, int y, u32 c)
{
	if (!(0 <= x && x < screen->w))
		return;
	if (!(0 <= y && y < screen->h))
		return;

	screen->fb[y * screen->w + x] = c;
}

// draw a filled rectangle on the screen
void
fillrect(int x, int y, int w, int h, u32 c)
{
	int px, py;

	for (py = y; py < y + h; py++) {
		for (px = x; px < x + w; px++) {
			setpixel(px, py, c);
		}
	}
}
