//
//  nkdpcmds.c
//  NotKDPShell
//
//  Created by Sem Voigtländer on 3/1/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <dlfcn.h>

#include <mach/mach.h>

#include "mshell_cmd_bundle.h"
#include "mshell_basic_cmds.h"
#include "mshell_file_cmds.h"
#include "mshell_adv_cmds.h"
#include "mshell_dev_cmds.h"
#include "mshell_kern_cmds.h"

#include "../Primitives/Userland/remotecall.h"
#include "../Primitives/Userland/remotemem.h"

#include "ANSIColors.h"


struct kdpshell_options {
    uint16_t port;
    const char *ip;
    bool verbose;
} kdpshell_default_options = {
#ifdef KDPSHELL_PORT
    KDPSHELL_PORT,
#else
    1337,
#endif
#ifdef KDPSHELL_IP
    KDPSHELL_IP,
#else
    "127.0.0.1",
#endif
#ifdef KDPSHELL_VERBOSE
    KDPSHELL_VERBOSE,
#else
    true,
#endif
};

int kdpshell_cmd_exit(int nargs, char* args[])
{
    extern void term_kexecute(void);
    extern void term_kernel(void);
    extern void term_jelbrek(void);
    term_kernel();
    term_kernel();
    term_jelbrek();
    exit(0);
}
int kdpshell_cmd_help(int nargs, char* args[]);
int kdp_shell_entercshell(int nargs, char *args[]);
struct kdpshell_cmd {
    
    const char* name;
    int (*fp)(int argc, char *argv[]);
    const char* usage;
    
} kdpshell_cmds[] = {
    
    //Kernel memory commands
    {"kwrite", kdpshell_cmd_kmem_write, "kwrite [addr] [data] [size]\n"},
    {"kread", kdpshell_cmd_kmem_read, "kread [addr] [data] [size]\n"},
    {"kdump", kdpshell_cmd_kmem_dump, "kdump [addr] [size]\n"},
    {"kalloc", kdpshell_cmd_kmem_alloc, "kalloc [size]\n"},
    {"kfree", kdpshell_cmd_kmem_free, "kfree [addr] [size]\n"},
    {"kexec", kdpshell_cmd_kexec, "kexec [addr] [x0-x6]\n"},
    {"kinfo", kdpshell_cmd_kinfo, "kinfo\n"},
    {"slide", kdpshell_cmd_slide, "slide [unslid addr]\n"},
    {"unslide", kdpshell_cmd_unslide, "unslide [slid addr]\n"},
    
    
    
    //General UNIX commands
    {"id", kdpshell_cmd_id, "id\n"},
    {"whoami", kdpshell_cmd_whoami, "whoami\n"},
    {"printf", kdpshell_cmd_printf, "printf [format] [args]\n"},
    {"execvp", kdpshell_cmd_execvp, "execvp [/path/to/binary] [args]\n"},
    {"setenv", kdpshell_cmd_setenv, "setenv [key] [value]\n"},
    {"getenv", kdpshell_cmd_getenv, "getenv [key]\n"},
    {"ps", kdpshell_cmd_ps, "ps\n"},
    {"uname", kdpshell_cmd_uname, "uname [option]\n"},
    {"du", kdpshell_cmd_du, "du [/path/to/disk]\n"},
    {"df", kdpshell_cmd_df, "df\n"},
    {"xxd", kdpshell_cmd_xxd, "xxd [file]\n"},
    {"maomaniac", kdpshell_cmd_dump_macho, "maomaniac [/path/to/macho_file]\n"},
    {"kill", kdpshell_cmd_kill, "kill [pid]\n"},
    {"uptime", kdpshell_cmd_uptime, "uptime\n"},
    {"freemem", kdpshell_cmd_freemem, "freemem\n"},
    
    
    {"ls", kdpshell_cmd_ls, "ls [/path/to/file_or_dir]\n"},
    {"touch", kdpshell_cmd_touch, "touch [/path/to/file]\n"},
    {"rm", kdpshell_cmd_rm, "rm [/path/to/file]\n"},
    {"mkdir", kdpshell_cmd_mkdir, "mkdir [/path/to/directory]\n"},
    {"cd", kdpshell_cmd_cd, "cd [/path/to/directory]\n"},
    {"pwd", kdpshell_cmd_pwd, "pwd\n"},
    {"symlink", kdpshell_cmd_symlink, "symlink\n"},
    
    //Server commands
    {"ftpd", kdpshell_cmd_ftpstart, "ftpd [~port]\n"},
    
    //Thread and task commands
    {"tfp", kdpshell_cmd_tfp, "tfp [pid]\n"},
    {"uxpid", kdpshell_cmd_threads_pid, "uxpid [pid]\n"},
    {"fport", kdpshell_cmd_findmachport, "fport [port]\n"},
    {"tregions", kdpshell_cmd_tsregions, "tsregions [taskport].\n"},
    {"tsregs", kdpshell_cmd_thread_setregs, "tsregs [threadport] [x0-x28]\n"},
    {"tregs", kdpshell_cmd_thread_getstate, "tregs [threadport]\n"},
    {"tsxregs", kdpshell_cmd_thread_set_execregs, "tsxregs [threadport] [sp | lr | pc | cpsr]\n"},
    {"tskread", kdpshell_cmd_task_read, "tskread [task] [addr] [size]\n"},
    {"tskwrite", kdpshell_cmd_task_write, "tskwrite [task] [addr] [value] [size]\n"},
    {"exit", kdpshell_cmd_exit, "exit"},
    //{"tskalloc",kdpshell_cmd_task_alloc, "tskalloc [task] [size]\n"},
    //{"tskfree", kdpshell_cmd_task_free, "tskfree [task] [size]\n"},
    
    {"cshell", kdp_shell_entercshell, "cshell"},
    //{"bash", kdpshell_spawn_bash", "bash"},
    
    //Show help of all commands.
    {"help", kdpshell_cmd_help, "help\n"},
};

int kdpshell_ncmds = sizeof(kdpshell_cmds) / sizeof(kdpshell_cmds[0]);

int cshell_cmd_find_sym(int nargs, char *args[])
{
    if(nargs < 2) return 0;
    void *self = dlopen(NULL, RTLD_NOW);
    void* sym = dlsym(self, args[1]);
    printf("%s: %#llx", args[1], (uint64_t)sym);
    dlclose(self);
    return 0;
}

int cshell_cmd_remote_cstring(int nargs, char *args[])
{
    mach_port_t task = (mach_port_t)strtoul(args[1], NULL, 16);
    uint64_t remotestring = remote_alloc(task, sizeof(char) * strlen(args[2]));
    printf("string is at: %#llx\n", remotestring);
    return 0;
}

int cshell_cmd_remote_buffer(int nargs, char *args[])
{
    mach_port_t task = (mach_port_t)strtoul(args[1], NULL, 16);
    uint64_t remotebuffer = remote_alloc(task, strtoull(args[2], NULL, 10));
    printf("buffer is at: %#llx\n", remotebuffer);
    return 0;
}

int cshell_cmd_exec(int nargs, char *args[])
{
    thread_t thread = (thread_act_t)strtoul(args[1], NULL, 16);
    void *fptr = NULL;
    if(strstr(args[2], "0x"))
    {
        fptr = (void*)strtoull(args[2], NULL, 16);
    } else
    {
        void* handle = dlopen(NULL, RTLD_NOW);
        fptr = (void*)dlsym(handle, args[2]);
        if(!fptr) {
            dlclose(handle);
            return 1;
        }
        dlclose(handle);
        printf("%s is at: %#llx\n", args[2], (uint64_t)fptr);
    }
    arg_desc* REMOTE_ARGS[8] = {};
    for(int i = 3; i < nargs && i-3 < 8; i++)
    {
        REMOTE_ARGS[i] = REMOTE_LITERAL(strtoull(args[i], NULL, 16));
    }
    
    uint64_t ret = thread_call_remote(thread, fptr, nargs - 2, REMOTE_ARGS[0], REMOTE_ARGS[1], REMOTE_ARGS[2], REMOTE_ARGS[3], REMOTE_ARGS[4], REMOTE_ARGS[5], REMOTE_ARGS[6], REMOTE_ARGS[7]);
    printf("Remote thread returned: %#llx\n", ret);
    return 0;
}

bool cshell_will_exit = false;

int cshell_cmd_exit(int nargs, char* args[])
{
    cshell_will_exit = true;
    return 0;
}
int cshell_cmd_help(int argc, char* argv[])
{
    for(int i = 0; i < kdpshell_ncmds; i++)
    {
        printf("%s\n\tusage: %s",kdpshell_cmds[i].name, kdpshell_cmds[i].usage);
    }
    return 0;
}

struct cshell_cmd {
    const char* name;
    int (*fp)(int argc, char *argv[]);
    const char* usage;
} cshell_cmds[] = {
    
    {"findsym", cshell_cmd_find_sym, "findsym [symname]\n"},
    {"exec", cshell_cmd_exec, "exec [thread] [addr/symname] [arg1-arg8]\n"},
    {"cstr", cshell_cmd_remote_cstring, "cstr [task] [string]\n"},
    {"buf", cshell_cmd_remote_buffer, "buf [task] [size]\n"},
    {"exit", cshell_cmd_exit, "exit\n"},
    {"help", cshell_cmd_help, "help\n"}
};

int cshell_ncmds = sizeof(cshell_cmds) / sizeof(cshell_cmds[0]);
int kdp_shell_cmdparse(int nargs, char *args[]);
void* current_interpreter = kdp_shell_cmdparse;

int cshell_cmdparse(int nargs, char *args[])
{
    
    if(cshell_will_exit) {
        cshell_will_exit = false;
        current_interpreter = kdp_shell_cmdparse;
        return 0;
    }
    
    int ret = 0;
    bool found = false;
    
    for(int i = 0; i < cshell_ncmds && !found; i++)
    {
        if(!strcmp(cshell_cmds[i].name, args[0]))
        {
            ret = cshell_cmds[i].fp(nargs, args);
            found = true;
        }
    }
    
    if(!found)
    {
        printf("'%s' command not found.\n", args[0]);
    }
    
    return 0;
}


int kdpshell_cmd_help(int argc, char* argv[])
{
    for(int i = 0; i < kdpshell_ncmds; i++)
    {
        printf("%s\n\tusage: %s",kdpshell_cmds[i].name, kdpshell_cmds[i].usage);
    }
    return 0;
}

int kdp_shell_entercshell(int nargs, char *args[])
{
    current_interpreter = cshell_cmdparse;
    return 0;
}

int kdp_shell_cmdparse(int nargs, char *args[])
{
    
    int ret = 0;
    bool found = false;
    
    for(int i = 0; i < kdpshell_ncmds && !found; i++)
    {
        if(!strcmp(kdpshell_cmds[i].name, args[0]))
        {
            ret = kdpshell_cmds[i].fp(nargs, args);
            found = true;
        }
    }
    
    if(!found)
    {
        printf("'%s' command not found.\n", args[0]);
    }
    
    return 0;
}

