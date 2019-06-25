//
//  kdp_adv-cmds.c
//  noncereboot1131UI
//
//  Created by Sem Voigtländer on 2/16/19.
//  Copyright © 2019 Pwn20wnd. All rights reserved.
//

#include "mshell_adv_cmds.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <Foundation/Foundation.h>
#include <machomaniac/machomaniac.h>


int kdpshell_cmd_uname(int nargs, char *args[])
{
    struct utsname u = {};
    uname(&u);
    if(nargs == 1)
    {
        printf("%s\n", u.sysname);
        return 1;
    }
    
    if(nargs > 1)
    {
        
        //uname -a
        if(args[1][0] == '-' && args[1][1] == 'a')
        {
            printf("%s %s %s\n", u.nodename, u.machine, u.version);
        }
        
        //uname -s
        else if(args[1][0] == '-' && args[1][1] == 's')
        {
            printf("%s\n", u.sysname);
        }
        
        //uname -r
        else if(args[1][0] == '-' && args[1][1] == 'r')
        {
            printf("%s\n", u.release);
        }
        
        //uname -v
        else if(args[1][0] == '-' && args[1][1] == 'v')
        {
            printf("%s\n", u.version);
        }
        
        //uname -n
        else if(args[1][0] == '-' && args[1][1] == 'n')
        {
            printf("%s\n", u.nodename);
        }
        
        //uname -m
        else if(args[1][0] == '-' && args[1][1] == 'm')
        {
            printf("%s\n", u.machine);
        }
        
        //uname -?
        else if(args[1][0] == '-' && args[1][1] == '?')
        {
            printf("usage: uname [-amnrsv]\n");
        }
        
        //uname var(x)
        else
        {
            printf("usage: uname [-amnrsv]\n");
        }
        
    }
    return 0;
}

int kdpshell_cmd_ps(int nargs, char *args[])
{
    return 0;
}

int kdpshell_cmd_du(int nargs, char *args[])
{
    return 0;
}

int kdpshell_cmd_df(int nargs, char *args[])
{
    return 0;
}


int kdpshell_cmd_xxd(int nargs, char *args[])
{
    
    return 0;
}

int kdpshell_cmd_dump_macho(int nargs, char* args[])
{
    if(nargs < 1) return 1;
    parse_and_print_macho_file(args[1]);
    return 0;
}

int kdpshell_cmd_kill(int nargs, char* args[])
{
    if(nargs < 1) return 1;
    kill(atoi(args[1]), -9);
    return 0;
}

int kdpshell_cmd_uptime(int nargs, char* args[])
{
    double uptime = [[NSProcessInfo alloc] systemUptime];
    printf("We're up since: %d days %d hours and %d minutes.\n", (int)(uptime / 60 / 60 / 24), (int)(uptime / 60 / 60), (int)(uptime / 60));
    return 0;
}
