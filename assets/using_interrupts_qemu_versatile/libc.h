#define nelem(x) (sizeof(x) / sizeof(x[0]))

typedef char *va_list;
#define va_start(list, start) list = \
                                  (sizeof(start) < 4 ? (char *)((int *)&(start) + 1) : (char *)(&(start) + 1))
#define va_end(list) \
	USED(list)
#define va_arg(list, mode) \
	((sizeof(mode) == 1) ? ((list += 4), (mode *)list)[-4] : (sizeof(mode) == 2) ? ((list += 4), (mode *)list)[-2] : ((list += sizeof(mode)), (mode *)list)[-1])

int vsnprint(char *, ulong, char *, va_list);
int vprint(char *, va_list);
int snprint(char *, ulong, char *, ...);
int print(char *, ...);
int iprint(char *, ...);
int puts(char *);
int putchar(int);

ulong strlen(char *);
char *strcpy(char *, char *);

void *memcpy(void *, void *, ulong);
void *memmove(void *, void *, ulong);
void *memset(void *, int, ulong);

void abort(void);

void panic(char *, ...);

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
