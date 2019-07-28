//
//  SYS_ARM64.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "defeat.h"
#include <stdlib.h>
#define BOOL bool
#include <stdbool.h>
#include <unistd.h>
#include <mach/mach.h>
#include <jelbrek/jelbrek.h>
#include "kutils.h"
#include "devicetree.h"
#include "offsets.h"
#include "patchfinder.h"

#define TCR1_TG1SZ_REPLACE TCR_TG1_GRANULE_64KB

mach_vm_address_t soc_base = 0;             // System on Chip base address mapping
mach_vm_address_t amcc_base = 0;            // Base address of Apple Memory Cache Controller
mach_vm_address_t cpacr_gadget = 0;         // MSR CPACR gadget
mach_vm_address_t ttbr0_el1_gadget = 0;     // MSR TTBR0_EL1, x1 gadget
mach_vm_address_t ttbr1_el1_gadget = 0;     // MSR TTBR1_EL1, x1 gadget

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

void set_tlb(mach_vm_address_t addr, int level){
    
    if(!level) // Set firstlevel pagetable entries
    {
        if(!ttbr0_el1_gadget){
            find_sysgadgets();
        }
        printf("About to set Translation Table Base 0 to: %#llx\n", addr);
        sleep(1);
        Kernel_Execute(ttbr0_el1_gadget, addr, 0, 0, 0, 0, 0, 0);
    }
    else if(level == 1) // Set second level pagetable entries
    {
        if(!ttbr1_el1_gadget){
            find_sysgadgets();
        }
        printf("About to set Translation Table Base 1 to: %#llx\n", addr);
        sleep(1);
        Kernel_Execute(ttbr1_el1_gadget, addr, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        printf("Invalid tlb level, must be either 0 or 1.\n");
    }
    
}

void set_cpacr(mach_vm_address_t addr){ // Set co-processor active control register (Makes KPP angry)
    if(!cpacr_gadget){
        find_sysgadgets();
    }
    printf("About to violate cpacr with address: %#llx\n", addr);
    sleep(1);
    Kernel_Execute(cpacr_gadget, addr, 0, 0, 0, 0, 0, 0);
}

void set_tcr(uint64_t bit){
    uint64_t new_tcr = 0;
    new_tcr = (TCR1_TG1SZ_REPLACE << TCR_T1SZ_SHIFT) & TCR_TSZ_MASK;
    printf("Setting TCR to %#llx...\n", new_tcr);
    sleep(3);
    Kernel_Execute(0xFFFFFFF0070A797+slide, new_tcr, 0, 0, 0, 0, 0, 0);
}

