//
//  debugutils.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef debugutils_h
#define debugutils_h

#include <stdio.h>
#include <mach/mach.h>
#include "xnu-header.h"
boolean_t ml_set_interrupts_enabled(boolean_t enable);
boolean_t  ml_get_interrupts_enabled(void);
int ml_get_cpucount(void);
int ml_get_boot_cpu_number(void);
int ml_get_cpu_number(void);
void PE_initialize_platform(bool vm_initialized, mach_vm_address_t bootArgs);
dbgwrap_status_t ml_dbgwrap_halt_cpu_with_state(int cpu_index, uint64_t timeout_ns, dbgwrap_thread_state_t *state);
const char* ml_dbgwrap_strerror(dbgwrap_status_t status); //get string value for dbgwrap_status enum
void trap_set_cpustate(dbgwrap_thread_state_t userland_state);
void kernel_debug_string_simple(uint32_t eventid, const char *message);
mach_vm_address_t PE_arm_get_soc_base_phys(void);
void print_threadstate(dbgwrap_thread_state_t state);
#endif /* debugutils_h */
