//
//  mapphys.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/26/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef mapphys_h
#define mapphys_h
#define SDRAM_BASE        (0x800000000ULL)
#define SDRAM_BANK_LEN        (SDRAM_LEN)
#define SDRAM_BANK_COUNT    (1)
#define SDRAM_END        (SDRAM_BASE + SDRAM_LEN)

#define VROM_BASE        (0x100000000ULL)
#define VROM_BANK_LEN        (0x02000000ULL)
#define VROM_LEN        (0x00080000ULL)
#define SRAM_BASE        (0x180000000ULL)
#define VROM_RSVD        (0x00100000ULL)        // 512 KiB reserved for SecureROM testing, remaining 512 KiB for Data and book-keeping
#define SRAM_LEN        (0x00400000ULL)        // 4 MiB
#define SRAM_BANK_LEN        (SRAM_LEN)        // Starting with Elba and Cayman, SRAM should be in a 32 MB decode region

#define SRAM_BANK_COUNT        (1)
#define SRAM_END        (SRAM_BASE + SRAM_LEN)

#define IO_BASE            (0x200000000ULL)
#define IO_SIZE            (0x020000000ULL)

#define PCI_REG_BASE        (0x600000000ULL)
#define PCI_REG_LEN        (0x00C000000ULL)
#define PCI_CONFIG_BASE        (0x610000000ULL)
#define PCI_CONFIG_LEN        (0x010000000ULL)
#define PCI_32BIT_BASE        (0x7C0000000ULL)
#define PCI_32BIT_LEN        (0x002000000ULL)        // 32 MiB fits nicely into a 32 MiB L2 MMU block

#define SECUREROM_LOAD_ADDRESS    (VROM_BASE)

/* reserved for ASP/NVMe */
// NOTE ASP_SIZE is now defined by the platform or target makefile.
#define ASP_BASE        (SDRAM_END - ASP_SIZE)

#define CONSISTENT_DEBUG_SIZE    (0x00004000ULL)
#define TZ1_SIZE        (0x00080000ULL)

#include <stdio.h>
void do_physmain(void);
mach_vm_address_t ml_phys_read( mach_vm_address_t paddr);
void ml_phys_write(mach_vm_address_t paddr, unsigned int data);
mach_vm_address_t ml_io_map(mach_vm_address_t paddr, vm_size_t lenght);
mach_vm_address_t ml_static_ptovirt(mach_vm_address_t paddr);
mach_vm_address_t get_bootrom_type(void);
mach_vm_address_t IOMemoryDescriptor_new(void);
mach_vm_address_t IOMemoryDescriptorWithPhysicalAddress(mach_vm_address_t this, mach_vm_address_t address, uint64_t x, uint64_t y);
mach_vm_address_t IOMemoryDescriptorWithAddress(mach_vm_address_t this,  mach_vm_address_t address, uint64_t x, uint y);
mach_vm_address_t IOMemoryDescriptorMap(mach_vm_address_t memoryDescriptor, uint64_t nBytes);
mach_vm_address_t IOMemoryMapGetVirtualAddress(mach_vm_address_t memoryMap);
mach_vm_address_t kvtophys(mach_vm_address_t vaddr);
mach_vm_address_t phystokv(mach_vm_address_t paddr);
void bcopy_phys(mach_vm_address_t dst, mach_vm_address_t src, mach_vm_size_t size);
void kerneldebug_enter(void);
#endif /* mapphys_h */

