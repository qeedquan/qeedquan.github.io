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

void timerinit(void);
void delay(int);
void microdelay(int);

void schedevent(u32);
void event(void);

void trapinit(void);
void intrsoff(void);
void intrson(void);
void intrenable(int, void (*)(Ureg *, void *), void *, char *);

void timerintr(Ureg *, void *);
void timeroneintr(Ureg *, void *);
void inputinr(Ureg *, void *);

void cacheuwbinv(void);
void coherence(void);

void vectors(void);
void vtable(void);
u32 *setr13(int, u32 *);

int spllo(void);
int splhi(void);
int splx(int);

#define round(x, r) (((x) + ((r)-1)) & ~(r))
