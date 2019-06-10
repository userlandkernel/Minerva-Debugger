//
//  pwnvfspolicy.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/30/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef pwnvfspolicy_h
#define pwnvfspolicy_h

#include <stdio.h>
#include <mach/mach.h>
void cache_enter(mach_vm_address_t dvp, mach_vm_address_t vp, mach_vm_address_t cnp);
kern_return_t pwnvfs_make_appleinternal(void);
void vnode_update_identity(mach_vm_address_t vp, mach_vm_address_t dvp,  mach_vm_address_t name, int name_len, uint32_t name_hashval, int flags);
#endif /* pwnvfspolicy_h */
