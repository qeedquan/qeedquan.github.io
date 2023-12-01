#include "arm.h"
#include "io.h"

typedef struct Uart Uart;
typedef struct Clcd Clcd;
typedef struct Timer Timer;
typedef struct Input Input;
typedef struct Cursor Cursor;
typedef struct Rect Rect;
typedef struct Vctl Vctl;
typedef struct Ureg Ureg;

struct Uart {
	volatile u32 *r;
};

struct Clcd {
	volatile u32 *r;

	int w, h;
	u32 *fb;
};

struct Timer {
	volatile u32 *r;
};

struct Input {
	volatile u32 *r;
	bool ismouse;
};

struct Cursor {
	int x, y;
	int dx, dy;
	int w, h;
};

struct Rect {
	int x, y, w, h;
};

struct Vctl {
	int irq;
	void (*f)(Ureg *, void *);
	void *a;
	char *name;
};

struct Ureg {
	ulong r0;
	ulong r1;
	ulong r2;
	ulong r3;
	ulong r4;
	ulong r5;
	ulong r6;
	ulong r7;
	ulong r8;
	ulong r9;
	ulong r10;
	ulong r11;
	ulong r12;
	union {
		ulong r13;
		ulong sp;
	};
	union {
		ulong r14;
		ulong link;
	};
	ulong type;
	ulong psr;
	ulong pc;
};

#define MHZ 1000000
#define HZ 100

extern Uart *consuart;
extern Clcd *screen;
extern int scheduled;