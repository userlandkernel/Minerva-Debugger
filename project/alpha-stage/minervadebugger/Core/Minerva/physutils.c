#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#define BOOL bool
#include <jelbrek/jelbrek.h>
#include "kutils.h"
#include "physutils.h"
#include "offsets.h"

mach_vm_address_t ml_io_read(vm_offset_t phys_addr, vm_size_t size){
    return Kernel_Execute(SYMOFF(_ML_IO_READ), phys_addr, size, 0, 0, 0, 0, 0);
}

mach_vm_address_t ml_phys_read(mach_vm_address_t paddr){
    return (unsigned int)Kernel_Execute(SYMOFF(_ML_PHYS_READ), paddr, 0, 0, 0, 0, 0, 0);
}

uint64_t ml_phys_read_byte64(mach_vm_address_t paddr){
    return (uint64_t)Kernel_Execute(SYMOFF(_ML_PHYS_READ_BYTE_64), paddr, 0, 0, 0, 0, 0, 0);
}

uint64_t ml_phys_read_word64(mach_vm_address_t paddr)
{
    return (uint64_t)Kernel_Execute(SYMOFF(_ML_PHYS_READ_WORD_64), paddr, 0, 0, 0, 0, 0, 0);
}

void ml_phys_write(mach_vm_address_t paddr, unsigned int data) {
    Kernel_Execute(SYMOFF(_ML_PHYS_WRITE), paddr, 0, 0, 0, 0, 0, 0);
}

void ml_phys_write_byte64(mach_vm_address_t paddr, unsigned int data){
    Kernel_Execute(SYMOFF(_ML_PHYS_READ_BYTE_64), paddr, data, 0, 0, 0, 0, 0);
}

void ml_phys_write_word64(mach_vm_address_t paddr, unsigned int data){
    Kernel_Execute(SYMOFF(_ML_PHYS_READ_WORD_64), paddr, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t ml_static_ptovirt(mach_vm_address_t paddr) {
    return Kernel_Execute(SYMOFF(_ML_STATIC_PTOVIRT), paddr, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t ml_io_map(mach_vm_address_t paddr, vm_size_t lenght){
    return Kernel_Execute(SYMOFF(_ML_IO_MAP), paddr, lenght, 0, 0, 0, 0, 0);
}

mach_vm_address_t get_bootrom_type(){
    return Kernel_Execute(SYMOFF(IOPLATFORMEXPERTGETBOOTROMTYPE), 0, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t IOMemoryDescriptorInit(mach_vm_address_t memdesc){
    
    return Kernel_Execute(SYMOFF(IOMEMORYDESCRIPTORINITIALIZE), 0, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t IOMemoryDescriptor_new(void){
    mach_vm_address_t memdesc = Kernel_alloc(0x1000);
    Kernel_Execute(0xFFFFFFF007574228+slide, memdesc, 0, 0, 0, 0, 0, 0);
    printf("IOMemoryDescriptor: %#llx\n", memdesc);
    sleep(1);
    IOMemoryDescriptorInit(memdesc);
    printf("Initialized memory descriptor: %#llx\n", memdesc);
    return memdesc;
}

// Returns IOMemoryDescriptor on success
mach_vm_address_t IOMemoryDescriptorWithPhysicalAddress(mach_vm_address_t this, mach_vm_address_t address, uint64_t x, uint64_t y){
    mach_vm_address_t IOMemoryDescriptor_withPhysicalAddress = SYMOFF(IOMEMORYDESCRIPTORWITHPHYSICALADDRESS);
    return Kernel_Execute(IOMemoryDescriptor_withPhysicalAddress, this, address, x, y, 0, 0, 0);
}
mach_vm_address_t IOMemoryDescriptorWithAddress(mach_vm_address_t this,  mach_vm_address_t address, uint64_t x, uint y){
    mach_vm_address_t IOMemoryDescriptor_withAddress = SYMOFF(IOMEMORYDESCRIPTORWITHADDRESS);
    return Kernel_Execute(IOMemoryDescriptor_withAddress, this, address, x, y, 0, 0, 0);
}

// Returns IOMemoryMap on success
mach_vm_address_t IOMemoryDescriptorMap(mach_vm_address_t memoryDescriptor, uint64_t nBytes){
    mach_vm_address_t IOMemoryDescriptor_map = SYMOFF(IOMEMORYDESCRIPTORMAP);
    return Kernel_Execute(IOMemoryDescriptor_map, memoryDescriptor, nBytes, 0, 0, 0, 0, 0);
}

// Returns the virtual address for the map
mach_vm_address_t IOMemoryMapGetVirtualAddress(mach_vm_address_t memoryMap){
    mach_vm_address_t IOMemoryMap_getVirtualAddress = SYMOFF(IOMEMORYMAPGETPHYSICALADDRESS);
    return Kernel_Execute(IOMemoryMap_getVirtualAddress, memoryMap, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t kvtophys(mach_vm_address_t vaddr){
    return Kernel_Execute(SYMOFF(_KVTOPHYS), vaddr, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t phystokv(mach_vm_address_t paddr){
    return Kernel_Execute(SYMOFF(_PHYSTOKV), paddr, 0, 0, 0, 0, 0, 0);
}

void bcopy_phys(mach_vm_address_t dst, mach_vm_address_t src, mach_vm_size_t size){
    Kernel_Execute(SYMOFF(_BCOPY_PHYS), dst, src, size, 0, 0, 0, 0);
}


