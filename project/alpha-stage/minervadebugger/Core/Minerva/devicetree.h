//
//  devicetree.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef devicetree_h
#define devicetree_h

#include <stdio.h>
#include <mach/mach.h>
kern_return_t DTLookupEntry(mach_vm_address_t num, const char* name, mach_vm_address_t* entry);
kern_return_t DTGetProperty(mach_vm_address_t entry, const char* name, mach_vm_address_t* reg_prop, mach_vm_size_t* size);
#endif /* devicetree_h */
