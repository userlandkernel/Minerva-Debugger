//
//  mshell_kern_cmds.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/25/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "mshell_kern_cmds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/machvm.h>
#include "patchfinder.h"

#define BOOL bool

#include <jelbrek/jelbrek.h>
#include "kutils.h"


int kdpshell_cmd_unslide(int nargs, char *args[])
{
    if(nargs < 1) return 1;
    uint64_t val = strtoull(args[1], NULL, 16)-slide;
    printf("unslid: %#llx\n", val);
    
    return 0;
}

int kdpshell_cmd_slide(int nargs, char *args[])
{
    uint64_t val = strtoull(args[1], NULL, 16)+slide;
    printf("slid: %#llx\n", val);
    return 0;
}

int kdpshell_cmd_kmem_alloc(int nargs, char* args[])
{
    if(nargs < 2) return 1;
    uint32_t size = (uint32_t)strtoul(args[1], NULL, 10);
    mach_vm_address_t addr = kalloc(size);
    printf("allocated %d bytes, addr: %#llx\n", size, addr);
    return 0;
}

int kdpshell_cmd_kmem_free(int nargs, char* args[])
{
    if(nargs < 3) return 1;
    kfree(strtoull(args[1], NULL, 16), strtoul(args[2], NULL, 10));
    return 0;
}

int kdpshell_cmd_kmem_dump(int nargs, char* args[])
{
    if(nargs < 3) return 1;
    uint64_t kaddr = strtoull(args[1], NULL, 16);
    size_t size = strtoul(args[2], NULL, 10);
    char* data = malloc(size);
    bool success = KernelRead(kaddr, data, size);
    if(success){
        HexDump((uint64_t)&data, size);
        if(data)
            free(data);
    } else {
        printf("failed.\n");
    }
    return 0;
}

int kdpshell_cmd_kmem_read(int nargs, char* args[])
{
    if(nargs < 3) return 1;
    uint64_t kaddr = strtoull(args[1], NULL, 16);
    size_t size = strtoul(args[2], NULL, 10);
    uint64_t* data = malloc(size);
    bool success = KernelRead(kaddr, data, size);
    if(success){
        for(int i = 0; i < size; i+=sizeof(uint64_t)){
            printf("+%d: %#llx\n", i, (uint64_t)data[i]);
        }
        if(data)
            free(data);
    } else {
        printf("failed.\n");
        
    }
    return 0;
}

int kdpshell_cmd_kmem_write(int nargs, char* args[])
{
    if(nargs < 3) return 1;
    uint64_t kaddr = strtoull(args[1], NULL, 16);
    uint64_t val = strtoull(args[2], NULL, 16);
    size_t size = strtoul(args[3], NULL, 10);
    copyout(kaddr, (const void*)val, size);
    printf("Wrote: %d bytes from %#llx to %#llx\n", (int)size, val, kaddr);
    return 0;
}

int kdpshell_cmd_kexec(int nargs, char *args[])
{
    if(nargs < 1) return 1;
    mach_vm_address_t addr = strtoull(args[1], NULL, 16);
    uint64_t regs[7] = {};
    for(int i = 0; i < nargs; i++){
        regs[i] = strtoull(args[i+i], NULL, 16);
    }
    printf("About to call %#llx with %d arguments.\n", addr, nargs-2);
    uint64_t ret = Kernel_Execute(addr, regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6]);
    printf("exec returned: %#llx\n", ret);
    return 0;
}

int kdpshell_cmd_findmachport(int nargs, char* args[])
{
    mach_port_t port = (mach_port_t)strtoul(args[1], NULL, 16);
    if(MACH_PORT_VALID(port)){
        mach_vm_address_t addr = FindPortAddress(port);
        printf("found port: %#x at %#llx\n", port, addr);
    } else {
        printf("Invalid port.\n");
        return 1;
    }
    return 0;
}

int kdpshell_cmd_kinfo(int nargs, char *args[])
{
    printf("Kernel taskport: %#x\n", tfp0);
    printf("Kernel slide: %#llx\n", slide);
    printf("Kernel base: %#llx\n", kbase);
    printf("Kernel process: %#llx\n", proc_of_pid(0));
    printf("Process list: %#llx\n", Find_allproc());
    printf("Kernel pagemap: %#llx\n", find_kernel_pmap());
    printf("Kernel zonemap: %#llx\n", Find_zone_map_ref());
    printf("Kernel bootargs: %#llx\n", Find_bootargs());
    printf("Physical memory base: %#llx\n", find_gPhysBase());
    printf("\nHappy Patching!\n");
    return 0;
}


int kdpshell_pid_for_proc(int nargs, char*args[])
{
    if(nargs < 1) return 1;
    const char* procname = args[1];
    pid_t pid = pid_of_procName((char*)procname);
    printf("pid: %d\n", pid);
    return 0;
}

int kdpshell_cmd_tfp(int nargs, char* args[])
{
    if(nargs < 1){
        printf("usage: tfp [pid].\n");
    }
    pid_t pid = strtod(args[1], NULL);
    mach_port_t task = MACH_PORT_NULL;
    task_for_pid(tfp0, pid, &task);
    printf("pid %d, task: %#x\n", pid, task);
    return 0;
}

int kdpshell_cmd_threads_pid(int nargs, char* args[])
{
    if(nargs < 1)
    {
        printf("usage: threads_pid [pid]\n");
    }
    vm_map_t task = (uint32_t)strtoul(args[1], NULL, 16);
    thread_act_array_t threads = NULL;
    mach_msg_type_number_t nthreads = 0;
    int ret = task_threads(task, &threads, &nthreads);
    if(ret != KERN_SUCCESS) {
        printf("failed to get threads for task %#x: %s\n", task, mach_error_string(ret));
    }
    for(int i = 0; i < nthreads; i++){
        printf("thread %d: %#x\n", i, threads[i]);
    }
    return 0;
}

int kdpshell_cmd_task_write(int nargs, char* args[])
{
    
    if(nargs > 3){
        vm_map_t task = (uint32_t)strtoul(args[1], NULL, 16);
        mach_vm_address_t addr = strtoull(args[2], NULL, 16);
        vm_offset_t data = (uint64_t)strtoull(args[3], NULL, 16);
        mach_vm_write(task, addr, data, sizeof(data));
        printf("wrote %#lx to %#llx in task %#x\n", data, addr, task);
    } else {
        printf("usage: task_write [taskport] [addr] [value]\n");
        return 1;
    }
    return 0;
}

int kdpshell_cmd_task_read(int nargs, char* args[])
{
    if(nargs < 3) return 1;
    vm_map_t task = (vm_map_t)strtoul(args[1], NULL, 16);
    mach_vm_address_t addr = strtoull(args[2], NULL, 16);
    vm_size_t size = strtoul(args[3], NULL, 10);
    vm_offset_t* data = malloc(size);
    mach_msg_type_number_t dC = 0;
    memset(data, 0, size);
    kern_return_t err = mach_vm_read(task, addr, size, data, &dC);
    if(err == KERN_SUCCESS)
    {
        for(int i = 0; i < size; i+=sizeof(uint64_t))
        {
            printf("+%d: %#llx\n", i, (uint64_t)data[i]);
        }
        if(data) {
            free(data);
            
        }
    }
    //printf("reading %d bytes from %d", (int)size, (int)task);
    
    return 0;
}

int kdpshell_cmd_thread_setregs(int nargs, char* args[])
{
    _STRUCT_ARM_THREAD_STATE64 state = {};
    if(nargs < 2) return 1;
    thread_act_t thread = (thread_act_t)strtoul(args[1], NULL, 16);
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    int err = thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &count);
    
    if(err != KERN_SUCCESS){
        printf("Failed to get threadstate: %s\n", mach_error_string(err));
    }
    
    for(int reg = 0,  i = 2; i < nargs && reg < sizeof(state.__x) / sizeof(state.__x[0]); i++, reg++){
        state.__x[reg] = strtoull(args[i], NULL, 16);
    }
    
    err = thread_set_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, count);
    if(err != KERN_SUCCESS){
        printf("Failed to set threadstate: %s\n", mach_error_string(err));
    }
    
    return 0;
}

int kdpshell_cmd_thread_set_execregs(int nargs, char* args[])
{
    _STRUCT_ARM_THREAD_STATE64 state = {};
    if(nargs < 5) return 1;
    thread_act_t thread = (thread_act_t)strtoul(args[1], NULL, 16);
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    int err = thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &count);
    
    if(err != KERN_SUCCESS){
        printf("Failed to get threadstate: %s\n", mach_error_string(err));
    }
    
    state.__sp = strtoull(args[nargs], NULL, 16);
    state.__lr = strtoull(args[nargs-1], NULL, 16);
    state.__pc = strtoull(args[nargs-2], NULL, 16);
    
    err = thread_set_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, count);
    if(err != KERN_SUCCESS){
        printf("Failed to set threadstate: %s\n", mach_error_string(err));
    }
    
    return 0;
}


int kdpshell_cmd_thread_getstate(int nargs, char* args[])
{
    _STRUCT_ARM_THREAD_STATE64 state = {};
    if(nargs < 2) return 1;
    thread_act_t thread = (thread_act_t)strtoul(args[1], NULL, 16);
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    int err = thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&state, &count);
    
    if(err != KERN_SUCCESS){
        printf("Failed to get threadstate.\n");
    }
    
    for(int i = 0; i < sizeof(state.__x) / sizeof(state.__x[0]); i++){
        printf("x%d: %#llx\n", i, state.__x[i]);
    }
    printf("lr: %#llx\n", state.__sp);
    printf("pc: %#llx\n", state.__pc);
    printf("sp: %#llx\n", state.__sp);
    printf("cpacr: %#x\n", state.__cpsr);
    printf("pad: %#x\n", state.__pad);
    
    return 0;
}


int kdpshell_cmd_tsregions(int nargs, char* args[])
{
    if(nargs < 2) return 1;
    mach_port_t task = (mach_port_t)strtoul(args[1], NULL, 16);
    struct vm_region_basic_info_64 region;
    mach_msg_type_number_t region_count = VM_REGION_BASIC_INFO_COUNT_64;
    memory_object_name_t object_name = MACH_PORT_NULL; /* unused */
    mach_vm_size_t size = sysconf(_SC_PAGESIZE);
    mach_vm_address_t addr = 0x0;
    kern_return_t err;
    for(;;) {
        err = mach_vm_region(task, &addr, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&region, &region_count, &object_name);
        if(err != KERN_SUCCESS) {
            break;
        }
        
        printf("%#llx %s%s%s\n", addr, (region.protection & VM_PROT_READ) ? "r" : "-", (region.protection & VM_PROT_WRITE) ? "w" : "-", (region.protection) & VM_PROT_EXECUTE ? "x" : "-");
        addr += size;
        
    }
    return 0;
}
