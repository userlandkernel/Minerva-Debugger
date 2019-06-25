//
//  mshell_file_cmds.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/25/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "mshell_file_cmds.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <copyfile.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>


int kdpshell_cmd_ls(int nargs, char *args[])
{
    char *path = NULL;
    if(nargs < 2) {
        path = getwd(NULL);
    }
    else {
        path = args[1];
    }
    if(!path) {
        path = getwd(NULL);
    }
    printf("%s\n", [[[NSFileManager defaultManager] contentsOfDirectoryAtPath:[NSString stringWithUTF8String:path] error:nil] componentsJoinedByString:@"\n"].UTF8String);
    return 0;
}

int kdpshell_cmd_touch(int nargs, char *args[])
{
    
    const char* file = args[1];
    FILE* fp = fopen(file, "a+");
    fclose(fp);
    return 0;
}

int kdpshell_cmd_rm(int nargs, char *args[])
{
    if(nargs < 2) return 1;
    remove(args[1]);
    return 0;
}

int kdpshell_cmd_mkdir(int nargs, char *args[])
{
    if(nargs < 2) return 1;
    char* path = args[1];
    [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithUTF8String:path] withIntermediateDirectories:NO attributes:nil error:nil];
    return 0;
}

int kdpshell_cmd_cd(int nargs, char *args[])
{
    if(nargs < 2) return 1;
    chdir(args[1]);
    return 0;
}

int kdpshell_cmd_pwd(int nargs, char *args[])
{
    char path[MAXPATHLEN];
    getcwd((char*)path, MAXPATHLEN);
    printf("%s\n", path);
    return 0;
}

int kdpshell_cmd_symlink(int nargs, char *args[])
{
    if(nargs < 3) return 0;
    unlink(args[2]);
    symlink(args[1], args[2]);
    return 0;
}

int kdpshell_cmd_cat(int nargs, char *argv[])
{
    if(nargs < 2) return 0;
    FILE* input = fopen(argv[1], "r");
    if (!input) {
        printf("cat: Failed to open %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    int c;
    while ((c = fgetc(input)) != EOF) {
        putchar(c);
    }
    return 0;
}
/*
 int kdpshell_cmd_cp(int nargs, char *argv[])
 {
 copyfile_flags_t flags = COPYFILE_CLONE | COPYFILE_DATA;
 if(!file_exist((char*) )) {
 return -1;
 }
 if(file_exist((char*)[to UTF8String])) {
 return -1;
 }
 int success = copyfile([from UTF8String], [to UTF8String], NULL, flags);
 return success;
 }*/


