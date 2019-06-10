//
//  minerva.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/5/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "minerva.h"
#include <UIKit/UIKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>

#include <sys/proc.h>
#include <sys/syscall.h>
#include <sys/cdefs.h>
#include <sys/queue.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/lock.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <mach-o/loader.h>
#include <kernel/machhelpers.h>
#include <mach/mach.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDEventSystem.h>

#include <jelbrek/jelbrek.h>
#include "patchfinder.h"
#include "xnu-header.h"
#include "pwnvfspolicy.h"
#include "sbsuspend.h"
#include "serial.h"
#include "videoconsole.h"
#include "debugutils.h"
#include "physutils.h"
#include "kutils.h"
#include "offsets.h"

mach_vm_address_t cpacr_gadget = 0;
mach_vm_address_t ttbr0_el1_gadget = 0;
mach_vm_address_t ttbr1_el1_gadget = 0;

#define SUPER_PRINTF(fmt, ...){\
printf(fmt, __VA_ARGS__);\
serial_printf(fmt, __VA_ARGS__);\
}


void hexdump(uint8_t buffer[], int len)
{
    int HEXDUMP_LINE_LEN = 16;
    int i;
    char s[HEXDUMP_LINE_LEN+1];
    bzero(s, HEXDUMP_LINE_LEN+1);
    
    for(i=0; i < len; i++) {
        if (!(i%HEXDUMP_LINE_LEN)) {
            if (s[0])
                SUPER_PRINTF("[%s]",s);
            SUPER_PRINTF("\n%05x: ", i);
            bzero(s, HEXDUMP_LINE_LEN);
        }
        s[i%HEXDUMP_LINE_LEN]=isprint(buffer[i])?buffer[i]:'.';
        SUPER_PRINTF("%02x ", buffer[i]);
    }
    while(i++%HEXDUMP_LINE_LEN)
        SUPER_PRINTF("%s","   ");
    
    SUPER_PRINTF("[%s]\n", s);
}

void loosen_macpolicies(void){
    printf("Loosening MAC policies...\n");
    WriteAnywhere32(0xFFFFFFF0075FF710+slide, 0x0); // Patch mac policy vnode_enforce
    SUPER_PRINTF("So did we disable vnode_enforce? %s\n", ReadAnywhere32(0xFFFFFFF0075FF710+slide) == 0 ? "yes" : "nope :(");
}

// Uses my patchfinder extensions for finding system instructions
void find_sysgadgets(void){
    printf("Finding ARM64 system instructions...\n");
    cpacr_gadget = find_cpacr_write();                      //set coprocessor active control register
    minerva_info("cpacr at: %#llx\n", cpacr_gadget);
    
    ttbr0_el1_gadget = find_ttbr0_el1_write();
    minerva_info("msr ttbr0_el1, x0 at: %#llx\n", ttbr0_el1_gadget);    // set translation table base register 0

    ttbr1_el1_gadget = find_ttbr1_el1_write();
    minerva_info("msr ttbr1_el1, x0 at: %#llx\n", ttbr1_el1_gadget);    // set translation table base register 1
}

void set_tlb0(mach_vm_address_t addr){  // Set firstlevel pagetable entries
    printf("About to set Translation Table Base 0 to: %#llx\n", addr);
    sleep(1);
    Kernel_Execute(ttbr0_el1_gadget, addr, 0, 0, 0, 0, 0, 0);
}

void set_tlb1(mach_vm_address_t addr){ // Set second level pagetable entries
    printf("About to set Translation Table Base 1 to: %#llx\n", addr);
    sleep(1);
    Kernel_Execute(ttbr1_el1_gadget, addr, 0, 0, 0, 0, 0, 0);
}

void set_cpacr(mach_vm_address_t addr){ // Set co-processor active control register (Makes KPP angry)
    printf("About to violate cpacr with address: %#llx\n", addr);
    sleep(1);
    Kernel_Execute(cpacr_gadget, addr, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t kstring(const char *string){  // Dirty in-kernel string creation
    mach_vm_address_t kaddr = 0;
    size_t len = strlen(string);
    kaddr = Kernel_alloc(len*sizeof(char));
    copyout(kaddr, (void*)string, len);
    return kaddr;
}


// Doubt this works, but is supposed to dump SRAM
void dump_sram(void){
    printf("Dumping SRAM...\n");
    mach_vm_address_t sramvirt = Kernel_alloc(SRAM_BANK_LEN);       // Allocate virtual memory to copy sram to
    uint8_t dump[SRAM_BANK_LEN] = {};
    bcopy_phys(kvtophys(sramvirt), SRAM_BASE+slide, SRAM_BANK_LEN);
    if(sramvirt){
        copyin(&dump, sramvirt, SRAM_BANK_LEN);
        Kernel_free(sramvirt, SRAM_BANK_LEN);
        sramvirt = 0;
        hexdump(dump, SRAM_BANK_LEN);
    }
}

void clear_documentsdir(void){
    NSString* docsdir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    for(NSString* doc in [NSFileManager.defaultManager contentsOfDirectoryAtPath:docsdir error:nil]){
        printf("Clearing %s...\n", (char*)doc.UTF8String);
        [NSFileManager.defaultManager removeItemAtPath:[doc stringByAppendingPathComponent:doc] error:nil];
    }
}

// Our main stage
kern_return_t minerva_init(void)
{
    kern_return_t err = KERN_SUCCESS;
    minerva_info("Initializing minerva...\n", NULL);
    
    // Clear the old kernelcaches etc.
    clear_documentsdir();
    
    // Get the kernel port
    err = get_kernelport(&escalation);
    if(err) {
        minerva_error("Failed to get kernel taskport: %s.\n", mach_error_string(err));
        return err;
    }
    
    //Get kernel base and slide
    err = get_kbase_and_slide(&escalation);
    if(err) {
         minerva_error("Failed to get kernel slide and base: %s.\n", mach_error_string(err));
        return err;
    }
    
    // Initialize jelbreklib
    err = init_with_kbase(tfp0, kbase);
    if(err != KERN_SUCCESS)
    {
        minerva_error("Failed to initialize jelbreklib: %s.\n", mach_error_string(err));
    }
    
    // Allow us get tasks of processes
    // Allow us to use task_for_pid
    // Allow running unsigned code
    // Allow us to debug apps
    
    // Let us run without a sandbox restriction
    err = !unsandbox(getpid());
    if(err != KERN_SUCCESS)
    {
        minerva_error("Failed to escape the sandbox: %s.\n", mach_error_string(err));
    }
    
    // Give us root privileges
    err = !rootify(getpid());
    if(err != KERN_SUCCESS)
    {
        minerva_error("Failed to escalate to the root user: %s.\n", mach_error_string(err));
    }
    
    //Mark ourselves as platform binary
    platformize(getpid());

    // Copy kernel header and loadcommands
    bool magic = (ReadAnywhere32(kbase) == 0xfeedfacf);
    minerva_info("Kernel mach-o magic: %s!\n", magic ? "valid" : "invalid");
    if(!magic){
        err = KERN_FAILURE;
        minerva_error("Kernel magic is invalid. Exiting...\n", NULL);
        term_jelbrek();
        exit(1);
        return err;
    }
    minerva_info("Initializing patchfinder..\n",nil);
    init_kernel(kbase, NULL); // Initialize patchfinder with the kernelbase
    
    minerva_info("Unlocking the NVRAM...\n", nil);
    UnlockNVRAM(); // Unlock the NVRAM
    
    minerva_info("Loosening mac policies...\n", nil);
    loosen_macpolicies(); // Patch MAC policies to loosen security

    minerva_info("Finding system instructions...\n", nil);
    find_sysgadgets(); // Find all system gadgets
    
    minerva_info("Patching the virtual filesystem to have /AppleInternal...\n", nil);
  //  pwnvfs_make_appleinternal(); // PWN the virtual filesystem to have a /AppleInternal directory
    
    minerva_warn("(Re)Initializing platform...", nil);
    PE_initialize_platform(FALSE, 0); // Reinit :)
    
    minerva_info("Initializing serial output...\n", nil);
    
    console_init();
    
    serial_init();
    
    PE_initialize_platform(FALSE, 0); // Reinit platform, 0 = use bootArgs constant from offsets
  
    // Just some example of serial prints, might be UART only idk
    serial_print("\n");
    serial_print("Debugger is on\n");
    serial_print("Welcome to @userlandkernel's Serial Log implementation. Have fun!\n");
    
    
    // We can use KPRINT for printing kernel strings, but remember its pretty dirty as it uses strlen()
   // serial_kprint(PATCHOFF(BOOTARGS));
    serial_print("\n");
    serial_print("Initializing the serial output...\n");
   
    // Finally enable the garbage collector
    // serial_print("Turning the garbage collector for the console on...\n");
    // gc_enable(TRUE);
  
    // Finally enable video console
    serial_print("Turning the video console on...\n");
    vc_enable(TRUE);
 
    
    PE_init_console(0, kPEEnableScreen);      // Text should appear on-screen
    PE_init_console(0, kPEGraphicsMode);
    PE_init_console(0, kPETextScreen);
    
    serial_print("\n");
    serial_kprint(kstring("We out here\n"));
    
    // Print some kernel information to the serial console.
    serial_print("==========================================================\n");
    serial_print("Kernel: ");
    serial_kprint(SYMOFF(_VERSION));
    serial_print("\n");
    serial_printf("Boot CPU: %#x\n", ml_get_boot_cpu_number());
    serial_printf("Current CPU: %#x\n", ml_get_cpu_number());
    serial_printf("CPU count: %#x\n", ml_get_cpucount());
    serial_printf("Hardware interrupts: %s\n", ml_get_interrupts_enabled() ? "enabled" : "disabled");
    serial_printf("Kernel virtual base: %#llx\n", KernelBase);
    serial_printf("Panic debugging: %#x\n", ReadAnywhere32(PATCHOFF(ENABLE_PANICDBG)));
    serial_printf("SoC Physical base: %#llx\n", ReadAnywhere64(0xFFFFFFF007674040+slide));
    serial_printf("SoC Virtual base: %#llx\n", phystokv(ReadAnywhere64(0xFFFFFFF007674040+slide)));
    
    serial_print("==========================================================\n");
    
    term_kernel();
    term_jelbrek();
    
    //exit(0);
    return err;
    
}
