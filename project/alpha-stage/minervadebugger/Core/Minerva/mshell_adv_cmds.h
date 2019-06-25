//
//  kdp_adv-cmds.h
//  noncereboot1131UI
//
//  Created by Sem Voigtländer on 2/16/19.
//  Copyright © 2019 Pwn20wnd. All rights reserved.
//

#ifndef kdp_adv_cmds_h
#define kdp_adv_cmds_h

#include <stdio.h>
int kdpshell_cmd_uname(int nargs, char *args[]);
int kdpshell_cmd_ps(int nargs, char *args[]);
int kdpshell_cmd_du(int nargs, char *args[]);
int kdpshell_cmd_df(int nargs, char *args[]);
int kdpshell_cmd_xxd(int nargs, char *args[]);
int kdpshell_cmd_dump_macho(int nargs, char* args[]);
int kdpshell_cmd_kill(int nargs, char* args[]);
int kdpshell_cmd_uptime(int nargs, char* args[]);
#endif /* kdp_adv_cmds_h */
