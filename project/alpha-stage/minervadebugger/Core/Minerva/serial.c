//
//  serial.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "serial.h"
#include <stdlib.h>
#include <unistd.h>
#include <IOKit/IOKitLib.h>
#include <sys/sysctl.h>
#include "xnu-header.h"
#include "kutils.h"
#include "offsets.h"
#include "debugutils.h"

size_t serial_buffer_size = 0x4000;

// Initializes the serial_printf buffer size, overflow prevention is up to the programmer!!!
size_t serial_buffer_init(size_t size) {
    if(size){
        serial_buffer_size = size;
    }
    return serial_buffer_size;
}

void PE_init_printf(void){
    printf("Enabling printf logging...\n");
     Kernel_Execute(SYMOFF(_PE_INIT_PRINTF), TRUE, 0, 0, 0, 0, 0, 0); // Enable printf logging
}

bool serial_enable_log(void){
    
    return (bool)Kernel_Execute(SYMOFF(_PE_I_CAN_HAS_DEBUGGER), 0, 0, 0, 0, 0, 0, 0); // Check if we can really debug
}

void serial_init_shmcon(void){
    printf("Initializing SHM connection...\n");
    Kernel_Execute(SYMOFF(_SHMCON_INIT), 0, 0, 0, 0, 0, 0, 0); // Initialize shm (shared memory) connection
}

mach_vm_address_t get_ttymalloced(void){
    for(int i = 0; i < 0x2000; i++){
        uint64_t kptr = Kernel_Execute(SYMOFF(_TTYMALLOC), 0, 0, 0, 0, 0, 0, 0);
        if(kptr != 0xffffffffe00002c2){
            return kptr;
        }
    }
    return 0;
}

// Hacky trick to initialize the console tty
void kminit(void){
int dev_off = 188;
    printf("dev_off: %d\n", dev_off);
    uint64_t km_tty = SLIDADDR(0xFFFFFFF007672D70);
    uint64_t tty_struct = get_ttymalloced(); //this malloc is async, sadly.
    if(!tty_struct){
        printf("Failed to init KM terminal..\n");
        return;
    }
    printf("tty_struct: %#llx\n", tty_struct);
    sleep(3);
    uint64_t initialized = SLIDADDR(0xFFFFFFF007672D78);
    uint64_t tty_struct_dev = tty_struct + dev_off;
    WriteAnywhere32(tty_struct_dev, makedev(12, 0));
    WriteAnywhere64(km_tty, tty_struct);
    WriteAnywhereBool(initialized, TRUE);
}

void serial_init_uart(void){
    printf("Initializing UART...\n");
    Kernel_Execute(SYMOFF(_UART_INIT), 0, 0, 0, 0, 0, 0, 0); // Initialize serial UART output
}

void serial_keyboard_init(void){
    mach_vm_address_t thread = 0;
    kernel_thread_start_priority(SYMOFF(_SERIAL_KEYBOARD_START), 0, MAXPRI_KERNEL, &thread);
    printf("Serial keyboard running on thread %#llx (%#llx -> %#llx)\n", thread, ReadAnywhere64(thread), ReadAnywhere64(ReadAnywhere64(thread)));
}

void real_serial_init(void){
    Kernel_Execute(SYMOFF(_SERIAL_INIT), 0, 0, 0, 0, 0, 0, 0);
}

void serial_init(void){
    printf("Initializing serial output...\n");
    real_serial_init();
    printf("Initializing serial SHM output...\n");
    serial_init_shmcon(); // Enable SHM output
    printf("Initializing the serial UART interface...\n");
    serial_init_uart(); // Enable UART output
    printf("Initializing the serial keyboard...\n");
    serial_keyboard_init();
    printf("Switching Console Option Index back to 0...\n");
    WriteAnywhereBool(PATCHOFF(CONS_OPS_INDEX), 0);
    sleep(1);
    printf("Enabling serial output...\n");
    // WriteAnywhereBool(PATCHOFF(DISABLE_SERIAL_OUTPUT), 0); // KTRR :(
    sleep(1);
    printf("Enabling console output...\n");
    serial_enable_log() ? printf("We can now have a debugger.\n") : printf("We failed to enable the debugger.\n"); // Enable logging & debug
    
    PE_init_printf();
    WriteAnywhereBool(PATCHOFF(DISABLE_CONSOLE_OUTPUT), 0);
    serial_print("When this message is shown in UART we have Serial output :)\n");
    
}

// Function for printing a single string to the console
void serial_print(const char *msg){
    size_t len = strlen(msg);
    for(int i = 0; i < len; i++){ //foreach character in the string
        Kernel_Execute(SYMOFF(_SERIAL_PUTC), msg[i], 0, 0, 0, 0, 0, 0); //print it to the serial console
    }
}

// Function for printing a string at the given address to the console
void serial_kprint(mach_vm_address_t addr){
    size_t size = Kernel_Execute(SYMOFF(_STRLEN), addr, 0, 0, 0, 0, 0, 0);
    char* str = malloc(size);
    copyin((void*)str, addr, size);
    serial_print(str);
    if(str){
        free(str);
        str = NULL;
    }
}
