//
//  videoconsole.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef videoconsole_h
#define videoconsole_h
#include <stdio.h>
#include <stdbool.h>
#include <mach/mach.h>
#include "xnu-header.h"
static struct {
    char * buffer;
    int len;
    int used;
    char * write_ptr;
    char * read_ptr;
    decl_simple_lock_data(, read_lock);
    decl_simple_lock_data(, write_lock);
} console_ring;

#define CPU_CONS_BUF_SIZE 256
#define CPU_BUF_FREE_HEX 0xf2eec075

typedef struct console_buf {
    char * buf_base;
    char * buf_end;
    char * buf_ptr;
#define CPU_BUFFER_LEN (CPU_CONS_BUF_SIZE - 3 * (sizeof(char *)))
    char buf[CPU_BUFFER_LEN];
} console_buf_t;

void console_init(void);
void gc_enable(bool enable);
void vc_enable(bool enable);
void backboardd_screensetup(void);
void PE_init_console(mach_vm_address_t PE_Video, uint64_t mode);
#endif /* videoconsole_h */
