//
//  kdp_basic-cmds.c
//  noncereboot1131UI
//
//  Created by Sem Voigtländer on 2/16/19.
//  Copyright © 2019 Pwn20wnd. All rights reserved.
//

#include "mshell_basic_cmds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <mach/port.h>
#define BOOL bool
#include <jelbrek/jelbrek.h>

int kdpshell_cmd_id(int nargs, char *args[])
{
    printf("uid=%d(%s) gid=%d euid=%d egid=%d\n", getuid(), getlogin(), getgid(), getuid(), getegid());
    return 0;
}

int kdpshell_cmd_whoami(int nargs, char *args[])
{
    printf("%s\n", getlogin());
    return 0;
}

int kdpshell_cmd_printf(int nargs, char* args[])
{
    if(nargs < 1) return 1;
    printf(args[1]);
    return 0;
}

int kdpshell_cmd_execvp(int nargs, char *args[])
{
    if(nargs < 2) {
        printf("usage: execvp [path] [trustbin (true|false)]\n");
        return 1;
    }
    if(atoi(args[2])){
        trustbin(args[1]);
    }
    return execvp(args[1], args + 1);
}

int kdpshell_cmd_setenv(int nargs, char *args[])
{
    if(nargs < 2) return 1;
    return setenv(args[1], args[2], true);
}

int kdpshell_cmd_getenv(int nargs, char *args[])
{
    if(nargs < 1) return 1;
    char* env = getenv(args[1]);
    if(env){
        printf("%s = %s\n", args[1], env);
    } else {
        printf("failed.\n");
    }
    return 0;
}

int kdpshell_cmd_echo(int nargs, char *args[])
{
    int i;
    for (i = 1; i < nargs - 1; ++i)
        printf("%s ", args[i]);
    printf("%s\n", args[i]);
    return 0;
}

int kdpshell_cmd_wc(int nargs, char *args[])
{
    bool wordc = false;
    bool linec = false;
    bool bytec = false;
    //bool charc = false; //fuck unicode and wchar_t and whatnot
    bool longlen = false;
    unsigned words = 0;
    unsigned lines = 0;
    unsigned bytes = 0;
    //unsigned characters = 0;
    unsigned longest = 0;
    unsigned curlen = 0;
    FILE *input;
    if (*args[nargs-1] != '-') {
        input = fopen(args[nargs-1], "r");
        if (!input)
            printf("wc: Could not open %s. Exiting.\n", args[nargs-1]);
    }
    else
        input = stdin;
    
    int i;
    for (i = 1; i < nargs; ++i) {
        if (!strcmp(args[i], "-w"))
            wordc = true;
        else if (!strcmp(args[i], "-b"))
            bytec = true;
        else if (!strcmp(args[i], "-l"))
            linec = true;
        else if (!strcmp(args[i], "-L"))
            longlen = true;
    }
    if (!wordc && !linec && !bytec && !longlen)
        wordc = true; //default to word counting
    
    int c;
    bool inword = false;
    while ((c = getc(input)) != EOF) {
        ++curlen;
        ++bytes;
        if (c == ' ' || c == '\t' || c == '\n')
            inword = false;
        else {
            if (inword == false)
                ++words;
            inword = true;
        }
        if (c == '\n') {
            ++lines;
            if (curlen > longlen)
                longest = curlen;
            curlen = 0;
        }
    }
    if (wordc)
        printf("Words: %u\n", words);
    if (linec)
        printf("Lines: %u\n", lines);
    if (bytec)
        printf("Bytes: %u\n", bytes);
    if (longlen)
        printf("Longest line: %u\n", longest);
    return 0;
}

int kdpshell_cmd_cowsay(int nargs, char *args[])
{
    int i;
    
    if (nargs == 1)
        printf("< moOh >\n");
    for (i = 1; i < nargs; i++)
        if (i == 1)
            printf("/ %s \\\n", args[i]);
        else if (i == nargs - 1)
            printf("\\ %s /\n", args[i]);
        else
            printf("| %s |\n", args[i]);
    printf("  \\ ^__^\n");
    printf("    (oo)\\_______\n");
    printf("    (__)\\       )\\/\\\n");
    printf("        ||----w |\n");
    printf("        ||     ||\n");
    return (0);
}
