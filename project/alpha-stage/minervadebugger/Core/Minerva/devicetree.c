//
//  devicetree.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "devicetree.h"
#include <stdlib.h>
#include <stdbool.h>
#include <mach/mach.h>
#include "kutils.h"
#include "offsets.h"
kern_return_t DTLookupEntry(mach_vm_address_t num, const char* name, mach_vm_address_t* entry){
    mach_vm_address_t kentry = Kernel_alloc(sizeof(mach_vm_address_t));
    mach_vm_address_t kname = Kernel_alloc(strlen(name));
    copyout(kname, (void*)name, strlen(name));
    kern_return_t err = (kern_return_t)Kernel_Execute(SYMOFF(_DTLOOKUPENTRY), num, kname, kentry, 0, 0, 0, 0);
    *entry = kentry;
    //  Kernel_free(kname, strlen(name));
    return err;
}

kern_return_t DTGetProperty(mach_vm_address_t entry, const char* name, mach_vm_address_t* reg_prop, mach_vm_size_t* size){
    mach_vm_address_t kreg_name = Kernel_alloc(strlen(name));
    mach_vm_address_t kentry = Kernel_alloc(sizeof(DTEntry));
    copyout(kentry, &entry, sizeof(DTEntry));
    copyout(kreg_name, (void*)name, strlen(name));
    mach_vm_address_t kreg_prop = Kernel_alloc(sizeof(mach_vm_address_t));
    mach_vm_address_t ksize = Kernel_alloc(sizeof(mach_vm_size_t));
    kern_return_t err = (kern_return_t)Kernel_Execute(SYMOFF(_DTGETPROPERTY), kentry, kreg_name, kreg_prop, ksize, 0, 0, 0);
    *size = ReadAnywhere64(ksize);
    *reg_prop = kreg_prop;
    return err;
}

int DTFindEntry(const char *propName, const char *propValue, mach_vm_address_t *entryH){
    mach_vm_address_t kprop_name = Kernel_alloc(strlen(propName));
    mach_vm_address_t kprop_value = Kernel_alloc(strlen(propValue));
    mach_vm_address_t kentryH = Kernel_alloc(sizeof(mach_vm_address_t));
    copyout(kprop_name, &propName, strlen(propName));
    copyout(kprop_value, &propValue, strlen(propValue));
    int rv = (int)Kernel_Execute(SYMOFF(_DTFINDENTRY), kprop_name, kprop_value, kentryH, 0, 0, 0, 0);
    *entryH = kentryH;
    return rv;
}
