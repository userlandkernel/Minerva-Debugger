//
//  serial.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef serial_h
#define serial_h

#include <stdio.h>
#include <stdbool.h>
#include <mach/mach.h>

void PE_init_printf(void);

void kminit(void);

size_t serial_buffer_init(size_t size);

void serial_print(const char *msg);
void serial_kprint(mach_vm_address_t kaddr);
void serial_init_uart(void);
bool serial_enable_log(void);
void serial_init(void);

#define serial_printf(fmt, ...) {\
    char *serial_printf_tmp = malloc(serial_buffer_init(0));\
    sprintf(serial_printf_tmp, fmt, __VA_ARGS__);\
    serial_print(serial_printf_tmp);\
    free(serial_printf_tmp);\
    serial_printf_tmp=NULL;\
};

#define SUPER_PRINTF(fmt, ...){\
    printf(fmt, __VA_ARGS__);\
    serial_printf(fmt, __VA_ARGS__);\
}

#endif /* serial_h */
