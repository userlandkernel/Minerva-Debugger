//
//  debugutils.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "debugutils.h"
#include "kutils.h"
#include "serial.h"
#include "offsets.h"
#include <unistd.h>
#include <stdlib.h>

boolean_t ml_set_interrupts_enabled(boolean_t enable) {
    return (boolean_t)Kernel_Execute(SYMOFF(_ML_SET_INTERRUPTS_ENABLED), enable, 0, 0, 0, 0, 0, 0);
}
boolean_t  ml_get_interrupts_enabled(void) {
    return (bool)Kernel_Execute(SYMOFF(_ML_GET_INTERRUPTS_ENABLED), 0, 0, 0, 0, 0, 0, 0);
}

int ml_get_cpucount(){
    return (int)Kernel_Execute(SYMOFF(_ML_GET_CPU_COUNT), 0, 0, 0, 0, 0, 0, 0);
}

int ml_get_boot_cpu_number(){
    return (int)Kernel_Execute(SYMOFF(_ML_GET_BOOT_CPU_NUMBER), 0, 0, 0, 0, 0, 0, 0);
}

int ml_get_cpu_number(){
    return (int)Kernel_Execute(SYMOFF(_ML_GET_CPU_NUMBER), 0, 0, 0, 0, 0, 0, 0);
}


const char* ml_dbgwrap_strerror(dbgwrap_status_t status) {
    switch (status) {
        case DBGWRAP_ERR_SELF_HALT:        return "CPU attempted to halt itself";
        case DBGWRAP_ERR_UNSUPPORTED:        return "halt not supported for this configuration";
        case DBGWRAP_ERR_INPROGRESS:        return "halt in progress on another CPU";
        case DBGWRAP_ERR_INSTR_ERROR:        return "instruction-stuffing failure";
        case DBGWRAP_ERR_INSTR_TIMEOUT:        return "instruction-stuffing timeout";
        case DBGWRAP_ERR_HALT_TIMEOUT:        return "halt ack timeout, CPU likely wedged";
        case DBGWRAP_SUCCESS:            return "halt succeeded";
        case DBGWRAP_WARN_ALREADY_HALTED:    return "CPU already halted";
        case DBGWRAP_WARN_CPU_OFFLINE:        return "CPU offline";
        default:                return "unrecognized status";
    }
}

dbgwrap_status_t ml_dbgwrap_halt_cpu_with_state(int cpu_index, uint64_t timeout_ns, dbgwrap_thread_state_t *state) {
    printf("About to halt the cpu for %llu nanoseconds...\n", timeout_ns);
    return (dbgwrap_status_t)Kernel_Execute(SYMOFF(_ML_DBGWRAP_HALT_CPU_WITH_STATE), cpu_index, timeout_ns, (uint64_t)state, 0, 0, 0, 0);
}

void print_threadstate(dbgwrap_thread_state_t state){
    for(int i = 0; i < sizeof(state.__x) / sizeof(state.__x[0]); i++){
        printf("x%d = %#llx\n",i, state.__x[i]);
        serial_printf("x%d = %#llx\n",i, state.__x[i]);
    }
    printf("pc = %#llx\n", state.__pc);
    serial_printf("pc = %#llx\n", state.__pc);
    printf("lr = %#llx\n", state.__lr);
    serial_printf("lr = %#llx\n", state.__lr);
    printf("fp = %#llx\n", state.__fp);
    serial_printf("fp = %#llx\n", state.__fp);
    printf("sp = %#llx\n", state.__sp);
    serial_printf("sp = %#llx\n", state.__sp);
    printf("cpsr = %#x\n", state.__cpsr);
    serial_printf("cpsr = %#x\n", state.__cpsr);
}


void trap_set_cpustate(dbgwrap_thread_state_t userland_state){
    printf("Kernel is about to trap and restore the cpu state of cpu %d...\n", ml_get_boot_cpu_number());
    uint64_t kernel_state = Kernel_alloc(sizeof(dbgwrap_thread_state_t));
    copyout(kernel_state, &userland_state, sizeof(dbgwrap_thread_state_t));
    dbgwrap_status_t status = ml_dbgwrap_halt_cpu_with_state(ml_get_boot_cpu_number(), 2, (dbgwrap_thread_state_t*)kernel_state);
    printf("Kernel reports from trap: %s\n",ml_dbgwrap_strerror(status));
    bzero(&userland_state, sizeof(dbgwrap_thread_state_t));
    copyin(&userland_state, kernel_state, sizeof(dbgwrap_thread_state_t));
    print_threadstate(userland_state);
}

void kernel_debug_string_simple(uint32_t eventid, const char *message){
    mach_vm_address_t kmsg = Kernel_alloc(strlen(message));
    copyout(kmsg, (void*)message, strlen(message));
    Kernel_Execute(SYMOFF(_KERNEL_DEBUG_STRING_SIMPLE), eventid, kmsg, 0, 0, 0, 0, 0);
}

void kerneldebug_enter(void){
    printf("Entering the kernel debugger...\n");
    if(!ReadAnywhereBool(SYMOFF(_KDEBUG_ENABLE))){
        printf("We need to initialize the serial console first...\n");
        serial_init();
    }
    Kernel_Execute(SYMOFF(_KERNEL_DEBUG_ENTER), 0, 0, 0, 0, 0, 0, 0);
}

void PE_initialize_platform(bool vm_initialized, mach_vm_address_t bootArgs){
    printf("(Re)initializing platform with vm_initialized = %s...\n", vm_initialized ? "TRUE" : "FALSE");
    if(!bootArgs) {
        bootArgs = PATCHOFF(BOOTARGS);
        printf("Will use original boot-args at %#llx\n", bootArgs);
    } else {
        printf("Will use bootargs: %#llx\n", bootArgs);
    }
    Kernel_Execute(SYMOFF(_PE_INITIALIZE_PLATFORM), vm_initialized, bootArgs, 0, 0, 0, 0, 0);
}

void halt_all_cpus(void){
    printf("We will HALT pcu execution!!!\n");
    Kernel_Execute(0xFFFFFFF0071C274C+slide, 0, 0, 0, 0, 0, 0, 0);
}

void panic_trap_to_debugger() {
    Kernel_Execute(SYMOFF(_PE_ENTER_DEBUGGER), 0, 0, 0, 0, 0, 0, 0);
}

mach_vm_address_t PE_arm_get_soc_base_phys(void){
    return Kernel_Execute(SYMOFF(_PE_ARM_GET_SOC_BASE_PHYS), 0, 0, 0, 0, 0, 0, 0);
}
