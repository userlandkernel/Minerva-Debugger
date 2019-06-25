//
//  videoconsole.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "videoconsole.h"
#include "../Primitives/Userland/remoteports.h"
#include "../Primitives/Userland/remotemem.h"
#include "../Primitives/Userland/remotecall.h"
#include "debugutils.h"
#include "kutils.h"
#include "xnu-header.h"
#include "offsets.h"
#include "lorgnette.h"
#include <mach/machvm.h>
#include <mach-o/loader.h>
#include <stdlib.h>
#include <proc.h>
#include <dlfcn.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <mach/vm_map.h>
#include <pthread.h>

mach_port_t get_task_for_processname(char* processName){
    mach_port_t taskPort = MACH_PORT_NULL;
    kern_return_t err = KERN_SUCCESS;
    int pid = pid_of_procName(processName);
    if(pid == -1){
        return MACH_PORT_NULL;
    }
    err = task_for_pid(mach_task_self(), pid, &taskPort);
    if(err == KERN_SUCCESS && MACH_PORT_VALID(taskPort)){
        return taskPort;
    }
    else {
        return MACH_PORT_NULL;
    }
}

/*
kern_return_t remote_call(mach_port_t task, uint64_t pc, uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3){
    kern_return_t err = KERN_SUCCESS;
    thread_act_t thread = MACH_PORT_NULL;
    err = thread_create(task, &thread);
    if(err != KERN_SUCCESS && !MACH_PORT_VALID(thread)){
        return KERN_FAILURE;
    }
    arm_thread_state64_t ts = {};
    ts.__pc = pc;
    ts.__x[0] = x0;
    ts.__x[1] = x1;
    ts.__x[2] = x2;
    ts.__x[3] = x3;
    err = thread_set_state(thread, ARM_THREAD_STATE64, (thread_state_t)&ts, ARM_THREAD_STATE64_COUNT);
    if(err != KERN_SUCCESS){
        return KERN_FAILURE;
    }
    err = thread_resume(thread);
    sleep(1);
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    err =  thread_get_state(thread, ARM_THREAD_STATE64, (thread_state_t)&ts, &count);
    print_threadstate(ts);
    return err;
}
 */

void backboardd_screensetup(void){
    mach_port_t tp = get_task_for_processname("backboardd");
    printf("Backboardd taskport: %#x\n", tp);
    thread_act_t ctlThread = MACH_PORT_NULL;
    thread_act_array_t threads = NULL;
    mach_msg_type_number_t nThreads = 0;
    task_threads(tp, &threads, &nThreads);
    thread_create(tp, &ctlThread);
    thread_resume(ctlThread);
    for(int i = 0; i  < nThreads; i++){
        thread_suspend(threads[0]);
    }
    uint64_t imageCnt = thread_call_remote(ctlThread, _dyld_image_count, 0);
    printf("BackBoard loaded images (%d)\n", (int)imageCnt);
    for(int i = 0; i < imageCnt; i++){
        printf("%d: %#llx\n", i, thread_call_remote(ctlThread, _dyld_get_image_header, i));
    };;;
    printf("Screen should be off.\n");
}

void gc_enable(bool enable){
    printf("Enabling the graphical console..\n");
    Kernel_Execute(SYMOFF(_GC_ENABLE), enable, 0, 0, 0, 0, 0, 0);
}
void console_init(void){
    printf("Initializing console..\n");
    mach_vm_address_t thread = 0;
    kernel_thread_start_priority(SYMOFF(_CONSOLE_INIT), 0, MAXPRI_KERNEL, &thread);
}

void enable_console(void){
    printf("Patching Platform Expert to setup video console correctly.\n");
    __unused struct PE_Video NewVideo = {};
}

void vc_enable(bool enable){
    printf("Enabling video console..\n");
    Kernel_Execute(SYMOFF(_VC_ENABLE), enable, 0, 0, 0, 0, 0, 0);
}

void PE_init_console(mach_vm_address_t PE_Video, uint64_t mode){
    if(!PE_Video){
        PE_Video = 0xFFFFFFF007095CE8+slide;
    }
    Kernel_Execute(0xFFFFFFF007579514+slide, 0, PE_Video, mode, 0, 0, 0, 0);
}

