#include "types.h"
#include "memlayout.h"
#include "arm.h"
#include "mmu.h"

void jump_stack(void);
void load_pgtbl(u32 *, u32 *);

int set_me;

static void
print0(char *s)
{
	volatile u8 *p = (void *)UART0;
	for (; *s; s++)
		*p = *s;
}

// setup the boot page table: dev_mem whether it is device memory
static void
set_bootpgtbl(u32 virt, u32 phy, uint len, int dev_mem)
{
	u32 pde, *user_pgtbl, *kernel_pgtbl;
	int idx;

	user_pgtbl = (u32 *)_user_pgtbl;
	kernel_pgtbl = (u32 *)_kernel_pgtbl;

	// convert all the parameters to indexes
	virt >>= PDE_SHIFT;
	phy >>= PDE_SHIFT;
	len >>= PDE_SHIFT;

	for (idx = 0; idx < len; idx++) {
		pde = (phy << PDE_SHIFT);

		if (!dev_mem) {
			// normal memory, make it kernel-only, cachable, bufferable
			pde |= (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
		} else {
			// device memory, make it non-cachable and non-bufferable
			pde |= (AP_KO << 10) | KPDE_TYPE;
		}

		// use different page table for user/kernel space
		if (virt < NUM_UPDE) {
			user_pgtbl[virt] = pde;
		} else {
			kernel_pgtbl[virt] = pde;
		}

		virt++;
		phy++;
	}
}

void
start(void)
{
	// print out a message to let us know that we made it here
	// we can't use char* because the string generated will not
	// be accessible yet since the R12 has not been setup yet
	// and we haven't relocated to the kernel address yet
	// we put the string on the stack here so we can print it
	char msg[] = "Hello from start(), about to setup the MMU\n";
	print0(msg);

	// double map the memory required for paging
	// we do not map all the physical memory
	set_bootpgtbl(0, 0, INIT_KERNMAP, 0);
	set_bootpgtbl(KERNBASE, 0, INIT_KERNMAP, 0);

	// map the vector table
	set_bootpgtbl(VEC_TBL, 0, 1 << PDE_SHIFT, 0);

	// map the devices so we can use the devices when we we enable the MMU
	set_bootpgtbl(KERNBASE + DEVBASE, DEVBASE, DEV_MEM_SZ, 1);

	// load the page table
	load_pgtbl((u32 *)_kernel_pgtbl, (u32 *)_user_pgtbl);

	// we have set up the MMU now, jump to the kernel address space
	jump_stack();
}
