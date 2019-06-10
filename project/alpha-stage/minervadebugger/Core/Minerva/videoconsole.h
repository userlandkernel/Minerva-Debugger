//
//  videoconsole.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef videoconsole_h
#define videoconsole_h
#include <stdio.h>
#include <stdbool.h>
#include <mach/mach.h>
void console_init(void);
void gc_enable(bool enable);
void vc_enable(bool enable);
void PE_init_console(mach_vm_address_t PE_Video, uint64_t mode);
#endif /* videoconsole_h */
