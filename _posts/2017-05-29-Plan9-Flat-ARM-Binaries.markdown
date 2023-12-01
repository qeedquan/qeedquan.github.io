---
layout: post
title:  "How to create flat ARM binaries in Plan 9"
date:   2017-05-29
categories: plan9 arm binary
---
Flat binaries are program executables that do not contain an executable header such as ELF or PE. Flat binaries are loaded directly into memory and executed. 
Boot loaders and kernels are examples of software that are created as flat binaries for loading.

This tutorial will show how to create a flat ARM binary using the Plan 9 toolchain. We based our example code from a tutorial [here].

{% highlight c %}
/* test.c */

#define UART0 ((void*) 0x101f1000)

typedef unsigned long u32;

#define nelem(x) (sizeof(x)/sizeof(x[0]))

void
println(char *s)
{
	volatile u32 *p = UART0;
	for (; *s; s++)
		*p = *s;
	*p = '\n';
}

/* 
 * Test that we set everything up correctly in startup.s
 * We should be able to access strings if we did
 */

static char *str1 = "str1";

char str2[] = "str2";

char *strtab[] = {
	"tab1",
	"tab2",
	"tab3",
};

void
test(void)
{
	int i;

	println("Hello from Plan 9");
	println(str1);
	println(str2);
	for (i = 0; i < nelem(strtab); i++)
		println(strtab[i]);
}

{% endhighlight %}

{% highlight asm %}
/* startup.s */

TEXT start(SB), 1, $0
	/* 
	 * 5c will generate accesses to global strings using R12 (IP) 
	 * as a lookup register, so the linker has pre-defined this 
	 * address for us that we need to set for R12 on startup
	 */
	MOVW    $setR12(SB), R12

	/* setup stack and jump to C */
	MOVW    $0x20000, SP
	BL      test(SB)

	/* loop forever doing nothing */
loop:
	B       loop
{% endhighlight %}

Now we can compile and link test.c and startup.s using the following commands:
{% highlight shell %}
# Plan 9 commands

# compile the C code into an object file
5c test.c

# assemble the assembly code into an object file
5a startup.s

# link the object files together, the obj order passed to the linker is important here,
# since the link order starts at the text address and increases orderly
# -l:       don't load in the startup linkage code and other libraries
# -H n:     6 means create a flat binary with no header with padding, alternatively 0 can be used also
# -T addr:  the addresses that the code generated will be relative to this, since qemu loads the code at 0x10000, put it relative to there
# -R pad :  align the text segment to 4096 bytes, the data section will come after this padding
5l -o plan9.bin -l -H6 -T0x10000 -R4096 startup.5 test.5

# generate a standard a.out format so we can disassemble it to verify correctness
5l -o plan9.out -l -T0x10000 -R4096 startup.5 test.5

{% endhighlight %}

Now that we have generated the plan9.bin binary, the next step is to check if it is valid assembly.
Since Plan 9 does not have a simple command to dump the assembly, we will resort to a sleazy hack to get the disassembly. We use [aout2elf] to convert it to an ELF binary for objdump to understand.

{% highlight asm %}
# Unix commands

$ aout2elf plan9.out
$ arm-none-eabi-objdump -D plan9.elf 

plan9.elf:     file format elf32-littlearm


Disassembly of section .text:

00010000 <start>:
   10000:	e52de004 	push	{lr}		; (str lr, [sp, #-4]!)
   10004:	e59fc08c 	ldr	ip, [pc, #140]	; 10098 <test+0x4c>
   10008:	e3a0d802 	mov	sp, #131072	; 0x20000
   1000c:	eb00000e 	bl	1004c <test>
   10010:	eafffffe 	b	10010 <start+0x10>

00010014 <println>:
   10014:	e52de008 	str	lr, [sp, #-8]!
   10018:	e59f507c 	ldr	r5, [pc, #124]	; 1009c <test+0x50>
   1001c:	e1a04000 	mov	r4, r0
   10020:	e1d430d0 	ldrsb	r3, [r4]
   10024:	e3530000 	cmp	r3, #0
   10028:	0a000004 	beq	10040 <println+0x2c>
   1002c:	e0d420d1 	ldrsb	r2, [r4], #1
   10030:	e5852000 	str	r2, [r5]
   10034:	e1d430d0 	ldrsb	r3, [r4]
   10038:	e3530000 	cmp	r3, #0
   1003c:	1afffffa 	bne	1002c <println+0x18>
   10040:	e3a0100a 	mov	r1, #10
   10044:	e5851000 	str	r1, [r5]
   10048:	e49df008 	ldr	pc, [sp], #8

0001004c <test>:
   1004c:	e52de00c 	str	lr, [sp, #-12]!
   10050:	e59f0048 	ldr	r0, [pc, #72]	; 100a0 <test+0x54>
   10054:	ebffffee 	bl	10014 <println>
   10058:	e51c0fe8 	ldr	r0, [ip, #-4072]	; 0xfffff018
   1005c:	ebffffec 	bl	10014 <println>
   10060:	e59f003c 	ldr	r0, [pc, #60]	; 100a4 <test+0x58>
   10064:	ebffffea 	bl	10014 <println>
   10068:	e3a04000 	mov	r4, #0
   1006c:	e3540003 	cmp	r4, #3
   10070:	aa000006 	bge	10090 <test+0x44>
   10074:	e58d4008 	str	r4, [sp, #8]
   10078:	e59f2028 	ldr	r2, [pc, #40]	; 100a8 <test+0x5c>
   1007c:	e7920104 	ldr	r0, [r2, r4, lsl #2]
   10080:	ebffffe3 	bl	10014 <println>
   10084:	e59d4008 	ldr	r4, [sp, #8]
   10088:	e2844001 	add	r4, r4, #1
   1008c:	eafffff6 	b	1006c <test+0x20>
   10090:	e49df00c 	ldr	pc, [sp], #12
   10094:	eafffffe 	b	10094 <test+0x48>
   10098:	00011ffc 	strdeq	r1, [r1], -ip
   1009c:	101f1000 	andsne	r1, pc, r0
   100a0:	0001102c 	andeq	r1, r1, ip, lsr #32
   100a4:	0001100c 	andeq	r1, r1, ip
   100a8:	00011000 	andeq	r1, r1, r0
   100ac:	00000000 	andeq	r0, r0, r0

Disassembly of section .data:

00011000 <bdata>:
   11000:	0001101d 	andeq	r1, r1, sp, lsl r0
   11004:	00011022 	andeq	r1, r1, r2, lsr #32
   11008:	00011027 	andeq	r1, r1, r7, lsr #32

0001100c <str2>:
   1100c:	32727473 	rsbscc	r7, r2, #1929379840	; 0x73000000
   11010:	00000000 	andeq	r0, r0, r0

00011014 <str1>:
   11014:	00011018 	andeq	r1, r1, r8, lsl r0

00011018 <.string>:
   11018:	31727473 	cmncc	r2, r3, ror r4
   1101c:	62617400 	rsbvs	r7, r1, #0, 8
   11020:	61740031 	cmnvs	r4, r1, lsr r0
   11024:	74003262 	strvc	r3, [r0], #-610	; 0xfffffd9e
   11028:	00336261 	eorseq	r6, r3, r1, ror #4
   1102c:	6c6c6548 	cfstr64vs	mvdx6, [ip], #-288	; 0xfffffee0
   11030:	7266206f 	rsbvc	r2, r6, #111	; 0x6f
   11034:	50206d6f 	eorpl	r6, r0, pc, ror #26
   11038:	206e616c 	rsbcs	r6, lr, ip, ror #2
   1103c:	00000039 	andeq	r0, r0, r9, lsr r0

{% endhighlight %}

Everything seems correct, so we run it through qemu and we should see the expected output.
{% highlight shell %}

# Unix commands
$ qemu-system-arm -M versatilepb -m 128M -nographic -kernel plan9.bin
Hello from Plan 9
str1
str2
tab1
tab2
tab3
# 'Ctrl-A' and then 'x' to exit
QEMU: Terminated
{% endhighlight %}


[here]: https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
[aout2elf]: https://github.com/qeedquan/debug/blob/master/9front/aout2elf.go
