//
//  videoconsole.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "videoconsole.h"
#include "kutils.h"
#include "xnu-header.h"
#include "offsets.h"

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
    Kernel_Execute(0xFFFFFFF0071A5168+slide, PE_Video, mode, 0, 0, 0, 0, 0);
}
