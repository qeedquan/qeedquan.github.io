#include "types.h"
#include "arm.h"
#include "memlayout.h"

#define nelem(x) (sizeof(x) / sizeof(x[0]))

void
putc(int c)
{
	volatile u8 *p = (void *)P2V(UART0);
	*p = c;
}

void
putln(char *s)
{
	for (; *s; s++)
		putc(*s);
	putc('\n');
}

void
puthex(uint v)
{
	char *hex = "0123456789ABCDEF";
	int i;

	for (i = sizeof(v) * 8 - 4; i >= 0; i -= 4)
		putc(hex[(v >> i) & 0xf]);
	putc('\n');
}

// test if we can access global variables now
static char bar[] = "bar";
char *foo = "foo";
static char *tab[] = {
    "tab1",
    "tab2",
    "tab3"};
int idx;
int *pidx;

extern int set_me;

struct test_struct {
	char *t1;
	char *t2;
	int t3;
} ts[] = {
    {"xxx", "yyy", 0x30},
    {"aaa", "yyy", 0x40},
    {"bbb", "yyy", 0x50},
    {"ccc", "yyy", 0x60},
    {"ddd", "yyy", 0x70},
    {"eee", "yyy", 0x80},
    {"fff", "yyy", 0x90},
    {"ggg", "yyy", 0x100},
    {"hhh", "yyy", 0x110},
};

void
kmain(void)
{
	putln("we are in kmain(), with the MMU enabled!");
	putln("running some tests to see if we can access strings and variables");
	putln(bar);
	putln(foo);
	for (idx = 0; idx < nelem(tab); idx++)
		putln(tab[idx]);
	puthex((uint)&idx);

	pidx = &idx;
	*pidx = 30;
	puthex(*pidx);

	set_me = 0xdeadbeef;
	puthex(set_me);
	puthex((uint)&set_me);

	for (set_me = 0; set_me < nelem(ts); set_me++) {
		putln(ts[set_me].t1);
		putln(ts[set_me].t2);
		puthex(ts[set_me].t3);
		puthex((uint)&ts[set_me].t1);
		puthex((uint)&ts[set_me].t2);
		puthex((uint)&ts[set_me].t3);
		putc('\n');
	}

	// we did not setup a return when we got to here so we don't want
	// to jump into space on return, just loop forever
	for (;;)
		;
}
