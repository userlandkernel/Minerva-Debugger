//
//  dxnu_rports.h
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef dxnu_rports_h
#define dxnu_rports_h

#include <stdio.h>
#include <mach/mach.h>
mach_port_name_t push_local_send(mach_port_t remote_thread_port, mach_port_t sright_to_push);
mach_port_t pull_remote_send(mach_port_t remote_thread_port, mach_port_name_t remote_send_right_name);
mach_port_t pull_remote_receive(mach_port_t remote_thread_port, mach_port_name_t remote_receive_right_name);
mach_port_t pull_remote_port(mach_port_t task_port, mach_port_name_t remote_port_name, mach_port_right_t disposition);    // eg MACH_MSG_TYPE_COPY_SEND
mach_port_name_t push_local_port(mach_port_t remote_task_port, mach_port_t port_to_push, mach_port_right_t disposition);
#endif /* dxnu_rports_h */
