// first kernel address
#define KERNBASE 0x80000000

// map the first 1 MB low memory containing kernel code.
#define INIT_KERNMAP 0x100000

// start of kernel data structures we need
#define edata_entry 0x2000

// stack for bootstrapping
#define svc_stktop 0x2000

// kernel page table address
#define _kernel_pgtbl 0x4000

// user page table address
#define _user_pgtbl 0x8000

// end of kernel data structures we need
#define end_entry 0x9000

#define P2V(a) ((a) + KERNBASE)
