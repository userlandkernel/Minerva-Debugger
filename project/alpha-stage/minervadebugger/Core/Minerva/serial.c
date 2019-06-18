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
    setlogin("root");
    printf("Setting baud rate to 115200...\n");
    WriteAnywhere32(PATCHOFF(GPE_SERIAL_BAUD), 115200); // Set baudrate

    printf("Enabling GNVME debugging...\n");
    WriteAnywhere32(PATCHOFF(GNVME_DEBUGFLAGS), TRUE); // Enable GNVME debugging
    
    printf("Setting KEXT Logging level...\n");
    WriteAnywhere32(0xFFFFFFF007763950+slide, 0x200000);
    
    printf("Enabling KEXT assertions...\n");
    WriteAnywhere32(0xFFFFFFF007602030+slide, 0x1);
    
    printf("Enabling verbose pointers...\n");
    WriteAnywhere32(0xFFFFFFF00760ED24+slide, 0x1);
    // Unknown debug
    // WriteAnywhere64(0xFFFFFFF007095C40+slide, 0x3); // 1 | 2, according to IDA (assertions etc.).
    // Sadly is in KTRR region
    
    printf("Enabling kernel debugging...\n");
    WriteAnywhere32(SYMOFF(_KDEBUG_ENABLE), KDEBUG_ENABLE_TRACE|KDEBUG_ENABLE_SERIAL|KDEBUG_ENABLE_ENTROPY|KDEBUG_ENABLE_PPT|KDEBUG_ENABLE_CHUD); // Enable all debugging
    
    printf("Enabling codesignature debugging...\n");
    WriteAnywhere32(SYMOFF(_CS_DEBUG), 1337); // Enable codesigning debugging
    
    printf("Enabling panic debugging...\n");
    WriteAnywhere32(PATCHOFF(ENABLE_PANICDBG), TRUE); // Enable panic debugging
    
    printf("Enabling IOKit debugging...\n");
    WriteAnywhere64(SYMOFF(_GIOKITDEBUG),kOSRegistryModsMode|
                    kIOTrackingBoot|
                    kIOTracking|kIOLogAttach|
                    kIOLogCatalogue|kIOLogConfig|
                    kIOLogDebugPower|kIOLogDTree|
                    kIOLogHibernate|kIOLogKextMemory|
                    kIOLogMemory|kIOLogMatch|
                    kIOLogMapping|kIOLogPower|
                    kIOLogProbe|kIOLogPMRootDomain|
                    kIOLogRegister|kIOLogStart|
                    kIOLogServiceTree|kIOLogTracePower|
                    kIOLogYield); // Enable IOKit debugging
    
    printf("Enabling debugger features...\n");
    WriteAnywhere32(SYMOFF(_DEBUGFLAG), DB_NMI|
                    DB_PRT_KDEBUG|
                    DB_PRT|DB_KPRT|
                    DB_ARP|DB_HALT|
                    DB_KDB|DB_KDP_BP_DIS|
                    DB_LOG_PI_SCRN|DB_DBG_POST_CORE|
                    DB_KERN_DUMP_ON_NMI); // Enable lots of kernel debugging
    
    return (bool)Kernel_Execute(SYMOFF(_PE_I_CAN_HAS_DEBUGGER), 0, 0, 0, 0, 0, 0, 0); // Check if we can really debug
}

void serial_init_uart(void){
    printf("Initializing UART...\n");
    Kernel_Execute(SYMOFF(_UART_INIT), 0, 0, 0, 0, 0, 0, 0); // Initialize serial UART output
}

void serial_init_shmcon(void){
    printf("Initializing SHM connection...\n");
    Kernel_Execute(SYMOFF(_SHMCON_INIT), 0, 0, 0, 0, 0, 0, 0); // Initialize shm (shared memory) connection
}

void serial_keyboard_init(void){
    // serial keyboard start
    mach_vm_address_t thread = 0;
    kernel_thread_start_priority(0xFFFFFFF00719FFBC+slide, 0, MAXPRI_KERNEL, &thread);
    printf("Serial keyboard running on thread %#llx (%#llx -> %#llx)\n", thread, ReadAnywhere64(thread), ReadAnywhere64(ReadAnywhere64(thread)));
}

void serial_init(void){
    printf("Initializing serial output...\n");
    PE_init_printf();
    serial_enable_log() ? printf("We can now have a debugger.\n") : printf("We failed to enable the debugger.\n"); // Enable logging & debug
    serial_init_uart(); // Enable UART output
    serial_init_shmcon(); // Enable SHM output
    serial_print("When this message is shown in UART we have Serial output :)\n");
    serial_keyboard_init();
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
