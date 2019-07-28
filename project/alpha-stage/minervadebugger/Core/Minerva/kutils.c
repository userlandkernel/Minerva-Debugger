//
//  kutils.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/26/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "kutils.h"
#include <stdlib.h>
#include <mach/machvm.h>
#include "physutils.h"
#include "offsets.h"
#include "debugutils.h"


escalation_data_t escalation = {};

// Dirty check whether an address is one that could be in the kernel
bool is_kaddr(mach_vm_address_t addr)
{
    return (addr & 0xffffffff00000000) == 0xfffffff000000000;
}

// Retrieves a kernel taskport from hsp4
kern_return_t get_kernelport(escalation_data_t* data)
{
    
    if(!data)
    {
        return KERN_INVALID_ARGUMENT;
    }
    
    return host_get_special_port(mach_host_self(), HOST_LOCAL_NODE, 4, &tfp0);
}

// Gets the kernel base and slide from the kernel task info as seen in unc0ver jailbreak
kern_return_t get_kbase_and_slide(escalation_data_t* data)
{
    
    if(!data)
    {
        return KERN_INVALID_ARGUMENT;
    }
    
    if(!MACH_PORT_VALID(tfp0))
    {
        return KERN_INVALID_RIGHT;
    }
    
    struct task_dyld_info dyld_info = { 0 };
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    
    if(task_info(tfp0, TASK_DYLD_INFO, (task_info_t)&dyld_info, &count) == KERN_SUCCESS)
    {
        if (is_kaddr(STATIC_KERNEL_BASE_ADDRESS + dyld_info.all_image_info_size))
        {
            data->kernel_base = STATIC_KERNEL_BASE_ADDRESS + dyld_info.all_image_info_size;
            data->kernel_slide = dyld_info.all_image_info_size;
            minerva_info("Kernel blob at: %#llx\n", dyld_info.all_image_info_addr); // Previously this was the kernelbase, there are kernel blobs now instead
            return KERN_SUCCESS;
        }
        else
        {
            minerva_warn("Kernel base or side are weird. Base = %#llx, slide = %#llx\n", dyld_info.all_image_info_addr, dyld_info.all_image_info_size);
            data->kernel_base = STATIC_KERNEL_BASE_ADDRESS + dyld_info.all_image_info_size;
            data->kernel_slide = dyld_info.all_image_info_size;
            minerva_info("Kernel blob at: %#llx\n", dyld_info.all_image_info_addr);
            return KERN_FAILURE;
        }
    }
    
    return KERN_SUCCESS;
}

// Copy data from the kernel into userland
void copyin(void* to, uint64_t from, size_t size) {
    mach_vm_size_t outsize = size;
    size_t szt = size;
    if (size > 0x1000) {
        size = 0x1000;
    }
    size_t off = 0;
    while (1) {
        mach_vm_read_overwrite(tfp0, off+from, size, (mach_vm_offset_t)(off+to), &outsize);
        szt -= size;
        off += size;
        if (szt == 0) {
            break;
        }
        size = szt;
        if (size > 0x1000) {
            size = 0x1000;
        }
        
    }
}

// Copy data from userland into the kernel
void copyout(uint64_t to, void* from, size_t size) {
    mach_vm_write(tfp0, to, (vm_offset_t)from, (mach_msg_type_number_t)size);
}

// Read primitives
uint64_t ReadAnywhere64(uint64_t addr) {
    uint64_t val = 0;
    copyin(&val, addr, sizeof(uint64_t));
    return val;
}

uint32_t ReadAnywhere32(uint64_t addr) {
    uint32_t val = 0;
    copyin(&val, addr, sizeof(uint32_t));
    return val;
}

uint16_t ReadAnywhere16(uint64_t addr){
    uint16_t val = 0;
    copyin(&val, addr, sizeof(uint16_t));
    return val;
}

uint8_t ReadAnywhere8(uint8_t addr){
    uint8_t val = 0;
    copyin(&val, addr, sizeof(uint8_t));
    return val;
}

bool ReadAnywhereBool(uint64_t addr){
    return (bool)ReadAnywhere8(addr);
}

// Write primitives
uint64_t WriteAnywhere64(uint64_t addr, uint64_t val) {
    copyout(addr, &val, sizeof(uint64_t));
    return val;
}

uint64_t WriteAnywhere32(uint64_t addr, uint32_t val){
    copyout(addr, &val, sizeof(uint32_t));
    return val;
}

uint64_t WriteAnywhere16(uint64_t addr, uint16_t val){
    copyout(addr, &val, sizeof(uint16_t));
    return val;
}

uint64_t WriteAnywhere8(uint64_t addr, uint8_t val){
    copyout(addr, &val, sizeof(uint8_t));
    return val;
}

uint64_t WriteAnywhereBool(uint64_t addr, bool val){
    copyout(addr, &val, sizeof(bool));
    return val;
}


void phys_copyin(void* to, uint32_t from, size_t size){
    copyin(to, phystokv(from), size);
}

void phys_copyout(uint32_t to, void* from, size_t size){
    copyout(phystokv(to), from, size);
}

// Creates an idle kernel thread with continuation (kernel function pointer) a parameter and given priority.
kern_return_t kernel_thread_create(mach_vm_address_t continuation, mach_vm_address_t parameter, integer_t priority, mach_vm_address_t *new_thread){
    mach_vm_address_t kern_new_thread = Kernel_alloc(sizeof(mach_vm_address_t));
    if(!continuation){
        continuation = SYMOFF(_THREAD_BOOTSTRAP_RETURN);
    }
    uint64_t err = Kernel_Execute(SYMOFF(_KERNEL_THREAD_CREATE), continuation, parameter, priority, kern_new_thread, 0, 0, 0);
    *new_thread = kern_new_thread;
    return (kern_return_t)err;
}

// Creates a running thread in the kernel.
kern_return_t kernel_thread_start_priority(mach_vm_address_t continuation, mach_vm_address_t parameter, integer_t priority, mach_vm_address_t *new_thread){
    mach_vm_address_t thread = Kernel_alloc(sizeof(mach_vm_address_t));
    kern_return_t err = (kern_return_t)Kernel_Execute(SYMOFF(_KERNEL_THREAD_START_PRIORITY), continuation, parameter, priority, thread, 0, 0, 0);
    *new_thread = thread;
    arm_thread_state64_t ts = {};
    copyin(&ts, ReadAnywhere64(thread), sizeof(arm_thread_state64_t));
    print_threadstate(ts);
    return err;
}

// Lower version with less validation for setting thread registers, can even operate on kernel threads
// However, it requires the thread to be suspended
kern_return_t machine_thread_set_state(mach_vm_address_t thread, thread_flavor_t flavor, arm_thread_state64_t tstate, mach_msg_type_number_t count){
    mach_vm_address_t kernel_tstate = Kernel_alloc(sizeof(arm_thread_state64_t));
    copyout(kernel_tstate, &tstate, sizeof(arm_thread_state64_t));
    return (kern_return_t)Kernel_Execute(SYMOFF(_MACHINE_THREAD_SET_STATE), thread, flavor, kernel_tstate, count, 0, 0, 0);
}
// Lower version with less validation for getting thread registers, can even operate on kernel threads
// However, it requires the thread to be suspended
kern_return_t machine_thread_get_state(mach_vm_address_t thread, thread_flavor_t flavor, arm_thread_state64_t tstate, mach_msg_type_number_t count){
    mach_vm_address_t kernel_tstate = Kernel_alloc(sizeof(arm_thread_state64_t));
    copyout(kernel_tstate, &tstate, sizeof(arm_thread_state64_t));
    return (kern_return_t)Kernel_Execute(SYMOFF(_MACHINE_THREAD_GET_STATE), thread, flavor, kernel_tstate, count, 0, 0, 0);
}

uint64_t physalloc(uint64_t size) {
    uint64_t ret = 0;
    mach_vm_allocate(tfp0, (mach_vm_address_t*) &ret, size, VM_FLAGS_ANYWHERE);
    return ret;
}
