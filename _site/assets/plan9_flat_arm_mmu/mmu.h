// ARMv6 has two page tables, we use one for kernel pages (TTBR1)
// and one for user pages (TTBR0). Memory addresses lower than
// 2^UADDR_BITS is translated by TTBR0, while higher memory is
// translated by TTBR1

// access permissions for page directory/table entries
// no access
#define AP_NA 0x00
// priviliege access, kernel: RW user: no access
#define AP_KO 0x01
// no write access from user, read allowed
#define AP_KUR 0x02
// full access
#define AP_KU 0x03

// cacheble memory
#define PE_CACHE (1 << 3)
// bufferable memory
#define PE_BUF (1 << 2)

// mask for page type
#define PE_TYPES 0x03
// use "section" type for kernel page directory
#define KPDE_TYPE 0x02
// use "coarse page table" for user page directory
#define UPDE_TYPE 0x01
// executable user page (subpage disable)
#define PTE_TYPE 0x02

// 1st-level or large (1MB) page directory (alway maps 1MB memory)
// shift how many bits to get the PDE index
#define PDE_SHIFT 20

// 2nd-level page table
// shift how many bits to get the PTE index
#define PTE_SHIFT 12

// maximum user-application memory, 256 MB
#define UADDR_BITS 28

// must have NUM_UPDE == NUM_PTE
// # of PDE for user space
#define NUM_UPDE (1 << (UADDR_BITS - PDE_SHIFT))
#define NUM_PTE (1 << (PDE_SHIFT - PTE_SHIFT))
