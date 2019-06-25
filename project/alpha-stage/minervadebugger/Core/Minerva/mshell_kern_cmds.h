//
//  kdp_kern-cmds.h
//  noncereboot1131UI
//
//  Created by Sem Voigtländer on 2/16/19.
//  Copyright © 2019 Pwn20wnd. All rights reserved.
//

#ifndef kdp_kern_cmds_h
#define kdp_kern_cmds_h

#include <stdio.h>


int kdpshell_cmd_findmachport(int nargs, char* args[]);
int kdpshell_cmd_unslide(int nargs, char *args[]);
int kdpshell_cmd_slide(int nargs, char *args[]);

int kdpshell_cmd_kmem_alloc(int nargs, char* args[]);
int kdpshell_cmd_kmem_free(int nargs, char* args[]);
int kdpshell_cmd_kmem_dump(int nargs, char* args[]);
int kdpshell_cmd_kmem_read(int nargs, char* args[]);
int kdpshell_cmd_kmem_write(int nargs, char* args[]);
int kdpshell_cmd_kexec(int nargs, char *args[]);
int kdpshell_cmd_kinfo(int nargs, char *args[]);

int kdpshell_pid_for_proc(int nargs, char*args[]);
int kdpshell_cmd_tfp(int nargs, char* args[]);
int kdpshell_cmd_threads_pid(int nargs, char* args[]);


int kdpshell_cmd_task_write(int nargs, char* args[]);
int kdpshell_cmd_task_read(int nargs, char* args[]);


int kdpshell_cmd_thread_getstate(int nargs, char* args[]);
int kdpshell_cmd_thread_set_execregs(int nargs, char* args[]);
int kdpshell_cmd_thread_setregs(int nargs, char* args[]);
int kdpshell_cmd_tsregions(int nargs, char* args[]);
#endif /* kdp_kern_cmds_h */
