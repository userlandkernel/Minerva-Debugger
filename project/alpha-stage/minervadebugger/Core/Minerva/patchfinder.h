#ifndef PATCHFINDER64_H_
#define PATCHFINDER64_H_

int init_kernel(uint64_t base, const char *filename);
void term_kernel(void);
enum { SearchInCore, SearchInPrelink };

// Fun part
uint64_t find_realhost(mach_vm_address_t addr);
uint64_t find_allproc(void);
uint64_t find_add_x0_x0_0x40_ret(void);
uint64_t find_copyout(void);
uint64_t find_bzero(void);
uint64_t find_bcopy(void);

uint64_t find_register_value(uint64_t where, int reg);
uint64_t find_reference(uint64_t to, int n, int prelink);
uint64_t find_strref(const char *string, int n, int prelink);
uint64_t find_gPhysBase(void);
uint64_t find_kernel_pmap(void);
uint64_t find_amfiret(void);
uint64_t find_ret_0(void);
uint64_t find_amfi_memcmpstub(void);
uint64_t find_sbops(void);
uint64_t find_lwvm_mapio_patch(void);
uint64_t find_lwvm_mapio_newj(void);
uint64_t find_ttbr1_el1_write(void);
uint64_t find_ttbr0_el1_write(void);

uint64_t find_entry(void);
const unsigned char *find_mh(void);

uint64_t find_cpacr_write(void);
uint64_t find_str(const char *string);
uint64_t find_amfiops(void);
uint64_t find_sysbootnonce(void);

int arm_pgshift(void);

extern uint64_t xnucore_base;
extern uint64_t xnucore_size;
extern uint64_t prelink_base;
extern uint64_t prelink_size;
extern uint64_t cstring_base;
extern uint64_t cstring_size;
extern uint64_t pstring_base;
extern uint64_t pstring_size;
extern uint64_t kerndumpbase;
extern uint64_t kernel_entry;
#endif
