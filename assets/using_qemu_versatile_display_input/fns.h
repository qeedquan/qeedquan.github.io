void uartputc(Uart *, int);
void uartinit(void);

void clcdinit(void);
void clcddisable(Clcd *);
void clcdenable(Clcd *);

void inputinit(void);
void pollinput(u32 *, u32 *);
void updatecursor(Cursor *, u32);

void setpixel(int, int, u32);
void fillrect(int, int, int, int, u32);
void filltexture(Texture *, Rect *, Rect *);

void timerinit(void);
void delay(int);
void microdelay(int);

#define round(x, r) (((x) + ((r)-1)) & ~(r))