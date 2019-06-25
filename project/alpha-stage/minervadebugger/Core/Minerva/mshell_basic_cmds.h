//
//  kdp_basic-cmds.h
//  noncereboot1131UI
//
//  Created by Sem Voigtländer on 2/16/19.
//  Copyright © 2019 Pwn20wnd. All rights reserved.
//

#ifndef kdp_basic_cmds_h
#define kdp_basic_cmds_h

#include <stdio.h>
int kdpshell_cmd_id(int nargs, char *args[]);
int kdpshell_cmd_whoami(int nargs, char *args[]);
int kdpshell_cmd_printf(int nargs, char* args[]);
int kdpshell_cmd_execvp(int nargs, char *args[]);
int kdpshell_cmd_setenv(int nargs, char *args[]);
int kdpshell_cmd_getenv(int nargs, char *args[]);
#endif /* kdp_basic_cmds_h */
