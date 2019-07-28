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
#include "patches.h"
#include "xnu-header.h"
#include "pwnvfspolicy.h"
#include "sbsuspend.h"
#include "serial.h"
#include "videoconsole.h"
#include "debugutils.h"
#include "physutils.h"
#include "kutils.h"
#include "offsets.h"
#include "kerneldec.h"
#include "devicetree.h"
#include "defeat.h"
#include "mshell.h"
#include "special_files.h"

void cleanup_documents(void)
{
    NSError *e = nil;
    NSString *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    NSArray* files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:path error:&e];
    for(NSString* file in files){
        printf("Got: %s\n", file.UTF8String);
    }
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

mach_vm_address_t kstring(const char *string){  // Dirty in-kernel string creation
    mach_vm_address_t kaddr = 0;
    size_t len = strlen(string);
    kaddr = Kernel_alloc(len*sizeof(char));
    copyout(kaddr, (void*)string, len);
    return kaddr;
}

/*
 * Secure RAM is cleared after the early boot.
 * It is not possible anymore to dump this from physical memory
 * Therefore this utility is useless
*/
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

void other_stuff(void){
    mach_vm_address_t options = 0;
    DTLookupEntry(0, "/options", &options);
    mach_vm_address_t isProdFused = 0;
    uint64_t size = 0;
    DTGetProperty(options, "94b73556-2197-4702-82a8-3e1337dafbfb:EffectiveProductionStatus", &isProdFused, &size);
    printf("is production fused: %#x\n", ReadAnywhere32(isProdFused));
}

void clear_documentsdir(void){
    NSString* docsdir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    for(NSString* doc in [NSFileManager.defaultManager contentsOfDirectoryAtPath:docsdir error:nil]){
        printf("Clearing %s...\n", (char*)doc.UTF8String);
        [NSFileManager.defaultManager removeItemAtPath:[doc stringByAppendingPathComponent:doc] error:nil];
    }
}

bool canRead(const char *file) {
    NSString *path = @(file);
    NSFileManager *fileManager = [NSFileManager defaultManager];
    return ([fileManager attributesOfItemAtPath:path error:nil]);
}

// Our main stage
kern_return_t minerva_init(void)
{
    kern_return_t err = KERN_SUCCESS;
    minerva_info("Initializing minerva...\n", NULL);
    
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
    
    // Update our session to root
    setlogin("root");
    
    //Mark ourselves as platform binary
    platformize(getpid());

    
    // Verify that we can read from the kernel base
    bool magic = (ReadAnywhere32(kbase) == 0xfeedfacf);
    minerva_info("Kernel mach-o magic: %s!\n", magic ? "valid" : "invalid");
    if(!magic){
        err = KERN_FAILURE;
        minerva_error("Kernel magic is invalid. Exiting...\n", NULL);
        term_jelbrek();
        exit(1);
        return err;
    }
    
    // Initialize the kernel patchfinder
    minerva_info("Initializing patchfinder..\n",nil);
    
    const char *kerndecomp_path = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/kernelcache.dec"].UTF8String;
    if(!canRead(kerndecomp_path)){
        
        if (!canRead(REAL_KERNELCACHE_PATH)) {
         return KERN_FAILURE;   // Are you sure we patched the sandbox?
        }
        
        // Open stream to real kernel
        FILE *kern_comp_real = fopen(REAL_KERNELCACHE_PATH, "rb");
        if(!kern_comp_real){
            return KERN_FAILURE;
        }
        
        // Open stream for decompressed kernel
        FILE *kern_decomp = fopen(kerndecomp_path, "wb+");
        
        // Decompress the kernel
        if(decompress_kernel(kern_comp_real, kern_decomp, NULL, true) != KERN_SUCCESS){
            return KERN_FAILURE;
        }
        
        // Dispose of any resources
        fclose(kern_decomp);
        fclose(kern_comp_real);
    }
    
    init_kernel(KernelRead, kbase, NULL); // Initialize patchfinder with the kernelbase

    minerva_info("Unlocking the NVRAM...\n", nil);
    UnlockNVRAM(); // Unlock the NVRAM
    
    minerva_info("Loosening mac policies...\n", nil);
    toggle_mac_vnode_enforce();

    minerva_info("Initializing serial output...\n", nil);
    
    // KernelWrite_32bits(0xFFFFFFF007095CF0+slide, 0);  // We can't patch data const (KTRR) :(
    // KernelWrite_64bits(SYMOFF(_PE_PANIC_DEBUGGING_ENABLED), 0x1);
    // KernelWrite_64bits(SYMOFF(_PE_ARM_DEBUG_PANIC_HOOK), SYMOFF(_NULLOP));
    printf("Initializing KM console device...\n");
    sleep(3);
    kminit();// must be fixed
    printf("Succeeded to initialize the KM console device!\n");
    
    
    serial_init();
    console_init();
    
    // Just some example of serial prints, might be UART only idk
    serial_print("\n");
    serial_print("Debugger is on\n");
    serial_print("Welcome to @userlandkernel's Serial Log implementation. Have fun!\n");
    
    minerva_info("Patching for the best default debug experience...\n", nil);
    toggle_kptr_stripping();
    toggle_nvme_debugging();
    toggle_panicdebugging();
    toggle_kext_assertions();
    set_kdebug(KDEBUG_MINERVA_DEFAULT);
    set_kext_logging(KEXTLOGGING_MINERVA_DEFAULT);
    set_csdebug(CSDEBUG_MINERVA_DEFAULT);
    set_iokit_debug(IOKIT_DEBUG_MINERVA_DEFAULT);
    set_kernel_debugflag(KERNEL_DEBUGFLAG_MINERVA_DEFAULT);
    minerva_info("Patches should have been applied!\n", nil);
    
    // We can use KPRINT for printing kernel strings, but remember its pretty dirty as it uses strlen()
    mach_vm_address_t bootargs = ReadAnywhere64(PATCHOFF(BOOTARGS));
    if(bootargs){
        serial_print("Boot arguments: ");
        serial_kprint(bootargs);
        serial_printf(" @ %#llx",bootargs);
        serial_print("\n");
    }
    serial_print("Initializing the serial output...\n");
    serial_print("\n");
    serial_kprint(kstring("We out here\n"));
    
    minerva_info("Finding system instructions...\n", nil);
    find_sysgadgets(); // Find all system gadgets
    
    //minerva_info("Patching the virtual filesystem to have /AppleInternal...\n", nil);
   // pwnvfs_make_appleinternal(); // PWN the virtual filesystem to have a /AppleInternal directory
    
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
    serial_printf("Kernel physical base: %#llx\n", kvtophys(KernelBase));
    serial_printf("Panic debugging: %#x\n", ReadAnywhere32(PATCHOFF(ENABLE_PANICDBG)));
    serial_printf("SoC Physical base: %#llx\n", ReadAnywhere64(SLIDADDR(0xFFFFFFF007674040)));
    serial_printf("SoC Virtual base: %#llx\n", phystokv(ReadAnywhere64(SLIDADDR(0xFFFFFFF007674040))));
    serial_printf("gPhysBase: %#llx\n", find_gPhysBase());
    serial_printf("TCR read access: %#llx\n", find_tcr_el1_read());
    serial_printf("TCR write access: %#llx\n", find_tcr_el1_write());
    serial_print("==========================================================\n");
    
    serial_print("\nAbout to fire the nukes...\n");
    
    uint64_t entryp = SLIDADDR(find_entry());
    serial_printf("Found entry: %#llx\n", entryp);
    uint64_t rvbar = entryp & (~0xFFF); // virtual base address register
    sleep(1);
    serial_printf("Found vbar register at: %#llx\n", rvbar);
    uint64_t cpul = find_register_value(rvbar+0x40, 1);
    sleep(1);
    serial_printf("Found cpul: %#llx\n", cpul);
    uint64_t optr = find_register_value(rvbar+0x50, 20);
    serial_printf("Found optr: %#llx\n", optr);
    sleep(1);
    uint64_t cpu_list = ReadAnywhere64(cpul - 0x10 /*the add 0x10, 0x10 instruction confuses findregval*/) - ReadAnywhere64(SLIDADDR(0xFFFFFFF007674040)) + phystokv(ReadAnywhere64(SLIDADDR(0xFFFFFFF007674040)));
    serial_printf("Found cpu list: %#llx\n", cpu_list);
    sleep(1);
    uint64_t cpu = ReadAnywhere64(cpu_list);
    serial_printf("Found cpu: %#llx\n", cpu);
    
    uint64_t shellcode = physalloc(0x4000);
    
    /*
     ldr x30, a
     ldr x0, b
     br x0
     nop
     a:
     .quad 0
     b:
     .quad 0
     none of that squad shit tho, straight gang shit. free rondonumbanine
     */
    
    WriteAnywhere32(shellcode + 0x100, 0x5800009e); /* trampoline for idlesleep */
    WriteAnywhere32(shellcode + 0x100 + 4, 0x580000a0);
    WriteAnywhere32(shellcode + 0x100 + 8, 0xd61f0000);
    
    WriteAnywhere32(shellcode + 0x200, 0x5800009e); /* trampoline for deepsleep */
    WriteAnywhere32(shellcode + 0x200 + 4, 0x580000a0);
    WriteAnywhere32(shellcode + 0x200 + 8, 0xd61f0000);
    
    sleep(1);
    serial_print("3. ");
    sleep(1);
    serial_print("2. ");
    sleep(1);
    serial_print("1. \n");

    kernel_boot_info_t boot_info = {};
    err =  host_get_boot_info(FakeHostPriv(), boot_info);
    serial_printf("boot info: %s\n", boot_info);
#ifdef test_threads
    mach_vm_address_t threads[10];
     char c = 'A';
    for(int i = 1; i < 10; i++){
        SUPER_PRINTF("SERIAL_PUTC kernel thread #%d\n\n", i);
        kernel_thread_start_priority(SYMOFF(_SERIAL_PUTC), c++, MAXPRI_KERNEL, &threads[i-1]);
    }
    serial_print("Thread test succeeded!\n");
#endif
    set_tcr(KERN_SUCCESS);
   // backboardd_screensetup();
   // find_socbase(); // Panics, DT function wrappers are broken lol
    printf("Preparing shell...\n");
    sleep(3);
   
    [NSThread detachNewThreadWithBlock:^{
        kern_return_t err = run_mshell(1337, NULL);
        SUPER_PRINTF("Running MSHELL on port 1337 %s", err == KERN_SUCCESS ? "succeeded!\n" : "failed.\n");
    }];
    
    /*uint64_t iomalloc_ptr = find_IOMalloc();
    for(int i = 0; i < 0x200; i++){
        uint64_t addr = Kernel_Execute(iomalloc_ptr, 87, 0, 0, 0, 0, 0, 0);
        printf("%d] %#llx which is in zone: NaN\n", i, addr);
        WriteAnywhere64(addr, 0x4141414141414141);
    }*/
    struct queue_entry {
        struct queue_entry    *next;        /* next element */
        struct queue_entry    *prev;        /* previous element */
    } __attribute__ ((aligned (8)));
    
    struct IOInterruptAccountingData {
        void *owner;
        struct queue_entry q;
        int interruptIndex;
        volatile uint64_t interruptStatistics[10] __attribute__((aligned(8)));
    };
    
    printf("We wil have to spray: %lu\n", sizeof(struct IOInterruptAccountingData));
    arm_thread_state64_t s = {};
    
    mach_vm_address_t thread = 0;
    kernel_thread_start_priority(SYMOFF(_SERIAL_GETC), 0, MAXPRI_KERNEL, &thread);
    thread = (ReadAnywhere64(thread));
    printf("Thread = %#llx\n", thread);
/*    sleep(1);
    for(int i = 0; i < 0xa; i++){
        s.__pc = SYMOFF(_SERIAL_PUTC);
        s.__x[0] = 0x41+i;
        printf("%s\n", mach_error_string(machine_thread_set_state(thread, ARM_THREAD_STATE64, s, ARM_THREAD_STATE64_COUNT)));
       
    }*/

   // term_kernel();
    //term_jelbrek();
   // exit(0);
    return err;
    
}
