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
#include "serial.h"
#include <mach/machvm.h>
#include <mach-o/loader.h>
#include <stdlib.h>
#include <proc.h>
#include <dlfcn.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <mach/vm_map.h>
#include <pthread.h>

#define GRAPHICS_MODE         1
#define FB_TEXT_MODE          2

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
    }
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

void run_videoconsole(void){
    PE_init_console(0, kPEEnableScreen);
    PE_init_console(0, kPEGraphicsMode);
    serial_print("Turning the graphical console on...\n");
    gc_enable(TRUE);
    
    serial_print("Turning the video console on...\n");
    vc_enable(TRUE); // Finally enable video console
}
