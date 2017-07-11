#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

Uart *consuart;

static void
vxputc(int ch, int *pos, char *str, ulong size, bool cons)
{
	if (cons)
		putchar(ch);
	else if (*pos < size)
		str[*pos] = ch;

	(*pos)++;
}

static void
vxputs(char *s, int *pos, char *str, ulong size, bool cons)
{
	for (; *s; s++)
		vxputc(*s, pos, str, size, cons);
}

static void
vxpnum(ulong val, int base, bool sign, bool prefix, int *pos, char *str, ulong size, bool cons)
{
	static char *numstr = "0123456789abcdef";

	char buf[16];
	int n;
	vlong v;

	v = val;
	if (sign && (long)val < 0) {
		vxputc('-', pos, str, size, cons);
		v = -((long)val);
	}

	if (prefix) {
		switch (base) {
		case 2:
			vxputs("0b", pos, str, size, cons);
			break;
		case 16:
			vxputs("0x", pos, str, size, cons);
			break;
		}
	}

	n = 0;
	do {
		buf[n++] = numstr[v % base];
		v /= base;
	} while (v > 0);

	while (--n >= 0)
		vxputc(buf[n], pos, str, size, cons);
}

static int
vxprint(char *fmt, va_list ap, char *str, ulong size, bool cons)
{
	int n, i;
	ulong u;
	char *p, *s, c;

	p = fmt;
	for (n = 0;;) {
		if (*p == '\0')
			break;

		if ((c = *p++) != '%') {
			vxputc(c, &n, str, size, cons);
			continue;
		}

		switch (*p++) {
		case 'u':
			u = va_arg(ap, uint);
			vxpnum(u, 10, false, false, &n, str, size, cons);
			break;
		case 'd':
			i = va_arg(ap, int);
			vxpnum(i, 10, true, false, &n, str, size, cons);
			break;
		case 'x':
			u = va_arg(ap, uint);
			vxpnum(u, 16, false, false, &n, str, size, cons);
			break;
		case 'p':
			u = va_arg(ap, ulong);
			if (u == 0)
				vxputs("(nil)", &n, str, size, cons);
			else
				vxpnum(i, 16, false, true, &n, str, size, cons);
			break;
		case 's':
			s = va_arg(ap, char *);
			vxputs(s, &n, str, size, cons);
			break;
		case '\0':
			return n;
		}
	}
	return n;
}

int
vsnprint(char *str, ulong size, char *fmt, va_list ap)
{
	return vxprint(fmt, ap, str, size, false);
}

int
vprint(char *fmt, va_list ap)
{
	return vxprint(fmt, ap, nil, 0, true);
}

int
snprint(char *str, ulong size, char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprint(str, size, fmt, ap);
	va_end(ap);
	return n;
}

int
print(char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
	return n;
}

int
iprint(char *fmt, ...)
{
	va_list ap;
	int x, n;

	x = splhi();
	va_start(ap, fmt);
	n = vprint(fmt, ap);
	va_end(ap);
	splx(x);
	return n;
}

int
putchar(int c)
{
	uartputc(consuart, c);
	return c;
}

int
puts(char *s)
{
	int n;

	for (n = 0; s[n]; n++)
		putchar(s[n]);
	putchar('\n');
	return n + 1;
}

ulong
strlen(char *s)
{
	ulong n;

	n = 0;
	while (s[n])
		n++;
	return n;
}

char *
strcpy(char *d, char *s)
{
	ulong n;

	for (n = 0; s[n]; n++)
		d[n] = s[n];
	return d;
}

void *
memcpy(void *d, void *s, ulong n)
{
	return memmove(d, s, n);
}

void *
memmove(void *d, void *s, ulong n)
{
	char *dp, *sp;

	dp = d;
	sp = s;
	if (sp < dp && sp + n > dp) {
		sp += n;
		dp += n;
		while (n-- > 0)
			*--dp = *--sp;
	} else {
		while (n-- > 0)
			*dp++ = *sp++;
	}
	return d;
}

void *
memset(void *s, int c, ulong n)
{
	ulong i;
	char *p;

	p = s;
	for (i = 0; i < n; i++)
		p[i] = c & 0xff;
	return s;
}

void
abort(void)
{
	for (;;)
		;
}

void
panic(char *fmt, ...)
{
	va_list ap;

	print("panic: ");
	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);
	print("\n");
	for (;;)
		;
}