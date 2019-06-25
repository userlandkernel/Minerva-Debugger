//
//  dxnu_rmem.h
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef dxnu_rmem_h
#define dxnu_rmem_h

#include <stdio.h>
#include <stdint.h>

// two basic bootstrappers which only need a thread port for r/w 64
uint64_t thread_r64(mach_port_t thread_port, uint64_t addr);
uint64_t thread_w64(mach_port_t thread_port, uint64_t addr, uint64_t value);

void* thread_rstr(mach_port_t thread_port, uint64_t addr);

void test_thread_rw(mach_port_t thread_port);

// allocate a buffer in the remote process
uint64_t
remote_alloc(mach_port_t task_port,
             uint64_t size);

// free a buffer in the remote process
void
remote_free(mach_port_t task_port,
            uint64_t base,
            uint64_t size);

// allocate a buffer in the remote process and fill it with the given contents
uint64_t
alloc_and_fill_remote_buffer(mach_port_t task_port,
                             uint64_t local_address,
                             uint64_t length);

// read from the remote address to the local address
// local address must be the address of a buffer at least length bytes in size
void
remote_read_overwrite(mach_port_t task_port,
                      uint64_t remote_address,
                      uint64_t local_address,
                      uint64_t length);

void
remote_write(mach_port_t remote_task_port,
             uint64_t remote_address,
             uint64_t local_address,
             uint64_t length);
#endif /* dxnu_rmem_h */
