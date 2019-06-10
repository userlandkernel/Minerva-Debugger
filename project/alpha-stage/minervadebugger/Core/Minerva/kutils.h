//
//  kutils.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/26/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef kutils_h
#define kutils_h

#include <stdio.h>
#include <stdbool.h>
#include <mach/mach.h>
#ifndef BOOL
#define BOOL bool
#endif
#include <jelbrek/jelbrek.h>

typedef struct escalation_data
{
    mach_port_t kernel_port;
    mach_vm_address_t kernel_base;
    mach_vm_offset_t kernel_slide;
} escalation_data_t;

#define MINLOG_INFO "info"
#define MINLOG_DEBUG "debug"
#define MINLOG_WARN "warning"
#define MINLOG_ERROR "error"

#define minerva_log(level, fmt, args...) printf("[%s](%s): "fmt, level, __func__, args)
#define minerva_info(fmt, args...) minerva_log(MINLOG_INFO, fmt, args)
#define minerva_warn(fmt, args...) minerva_log(MINLOG_WARN, fmt, args)
#define minerva_error(fmt, args...) minerva_log(MINLOG_ERROR, fmt, args);

#define STATIC_KERNEL_BASE_ADDRESS 0xfffffff007004000

extern escalation_data_t escalation;

bool is_kaddr(mach_vm_address_t addr); // Credits: Brandon Azad
kern_return_t get_kernelport(escalation_data_t* data);
kern_return_t get_kbase_and_slide(escalation_data_t *escalation); // Credits: pwn20wnd

void copyin(void* to, uint64_t from, size_t size); // Credits: Luca Todesco
void copyout(uint64_t to, void* from, size_t size); // Credits: Luca Todesco
void phys_copyin(void* to, uint32_t from, size_t size);
void phys_copyout(uint32_t to, void* from, size_t size);

uint64_t ReadAnywhere64(uint64_t addr); // Credits: Luca Todesco
uint32_t ReadAnywhere32(uint64_t addr); // Credits: Luca Todesco
uint16_t ReadAnywhere16(uint64_t addr);
uint8_t ReadAnywhere8(uint8_t addr);
bool ReadAnywhereBool(uint64_t addr);

uint64_t WriteAnywhere64(uint64_t addr, uint64_t val); // Credits: Luca Todesco
uint64_t WriteAnywhere32(uint64_t addr, uint32_t val); // Credits: Luca todesco
uint64_t WriteAnywhere16(uint64_t addr, uint16_t val);
uint64_t WriteAnywhere8(uint64_t addr, uint8_t val);
uint64_t WriteAnywhereBool(uint64_t addr, bool val);

#define SReadAnywhere64(addr) ReadAnywhere64(addr+slide)
#define SReadAnywhere32(addr) ReadAnywhere32(addr+slide)
#define SReadAnywhere16(addr) ReadAnywhere16(addr+slide)
#define SReadAnywhere8(addr) ReadAnywhere8(addr+slide)
#define SReadAnywhereBool(addr) ReadAnywhereBool(addr+slide)

#define SWriteAnywhere64(addr,val) WriteAnywhere64(addr+slide,val)
#define SWriteAnywhere32(addr,val) WriteAnywhere32(addr+slide,val)
#define SWriteAnywhere16(addr,val) WriteAnywhere16(addr+slide,val)
#define SWriteAnywhere8(addr,val) WriteAnywhere8(addr+slide,val)
#define SWriteAnywhereBool(addr,val) WriteAnywhereBool(addr+slide,val)



kern_return_t kernel_thread_start_priority(mach_vm_address_t continuation, mach_vm_address_t parameter, integer_t priority, mach_vm_address_t *new_thread);

kern_return_t machine_thread_set_state(mach_vm_address_t thread, thread_flavor_t flavor, arm_thread_state64_t tstate, mach_msg_type_number_t count);

kern_return_t machine_thread_get_state(mach_vm_address_t thread, thread_flavor_t flavor, arm_thread_state64_t tstate, mach_msg_type_number_t count);

#define vm_address_t mach_vm_address_t
#define tfp0 escalation.kernel_port
#define slide escalation.kernel_slide
#define kbase escalation.kernel_base

#endif /* kutils_h */
