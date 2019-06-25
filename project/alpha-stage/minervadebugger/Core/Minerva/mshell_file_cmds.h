//
//  mshell_file_cmds.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/25/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef mshell_file_cmds_h
#define mshell_file_cmds_h

#include <stdio.h>
int kdpshell_cmd_ls(int nargs, char *args[]);
int kdpshell_cmd_touch(int nargs, char *args[]);
int kdpshell_cmd_rm(int nargs, char *args[]);
int kdpshell_cmd_mkdir(int nargs, char *args[]);
int kdpshell_cmd_cd(int nargs, char *args[]);
int kdpshell_cmd_pwd(int nargs, char *args[]);
int kdpshell_cmd_symlink(int nargs, char *args[]);
#endif /* mshell_file_cmds_h */
