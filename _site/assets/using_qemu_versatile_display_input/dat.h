typedef struct Uart Uart;
typedef struct Clcd Clcd;
typedef struct Timer Timer;
typedef struct Input Input;
typedef struct Cursor Cursor;
typedef struct Texture Texture;
typedef struct Rect Rect;

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

struct Texture {
	int w, h;
	u32 *p;
};

struct Rect {
	int x, y, w, h;
};

#define MHZ 1000000

extern Uart *consuart;
extern Clcd *screen;
