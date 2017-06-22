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

// copy a pixel buffer to the screen
void
filltexture(Texture *t, Rect *d, Rect *s)
{
	Rect dp, sp;
	int x, y, w, h, tx, ty;

	dp = (Rect){0, 0, screen->w, screen->h};
	sp = (Rect){0, 0, t->w, t->h};
	if (d)
		dp = *d;
	if (s)
		sp = *s;

	w = min(dp.w, sp.w);
	h = min(dp.h, sp.h);
	for (ty = sp.y, y = dp.y; y < dp.y + h; y++, ty++) {
		for (tx = sp.x, x = dp.x; x < dp.x + w; x++, tx++) {
			if (!(0 <= tx && tx < t->w))
				continue;
			if (!(0 <= ty && ty < t->h))
				continue;

			setpixel(x, y, t->p[ty * t->w + tx]);
		}
	}
}
