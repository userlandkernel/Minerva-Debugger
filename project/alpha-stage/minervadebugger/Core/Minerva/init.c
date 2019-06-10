//
//  init.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/4/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "init.h"
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
const char *minerva_copyright =
"Copyright (c) 2019\n\t"
"Sem Voigtländer <info@kernelprogrammer.com>. "
"All rights reserved.\n\n";

const int version_major = MINERVA_VERSION_MAJOR;
const int version_minor = MINERVA_VERSION_MINOR;
const int version_revision = MINERVA_REVISION;
const int version_stage = MINERVA_STAGE;
const int version_prerelease_level = MINERVA_PRERELEASE_LEVEL;
const char version_variant[256] = MINERVA_VERSION_VARIANT;
const char osbuilder[256] = MINERVA_OSBUILDER;
const char osrelease[256] = MINERVA_OSRELEASE;
const char ostype[256] = MINERVA_OSTYPE;
const char *compile_date = __DATE__;
const char *compile_time = __TIME__;
struct minerva_utsname minerva_info = {};

void __attribute__((constructor)) minerva_init_core(void){
    memset(&minerva_info, 0, sizeof(minerva_info));
    struct utsname u = {};
    uname(&u);
    minerva_info.machine = u.machine;
    minerva_info.nodename = u.nodename;
    minerva_info.sysname = (char*)ostype;
    minerva_info.release = (char*)osrelease;
    asprintf(&minerva_info.version, "%s Debugger Version %d.%d.%d: %s %s %s:%s %s", ostype, version_major, version_minor, version_revision, compile_date, compile_time, osbuilder, osrelease, minerva_info.machine);
    printf("%s\n\n", minerva_info.version);
    init_minerva_offsets();
    printf("Printing first few offsets...\n");
    for(int i = 0; i < 5; i++){
        printf("off %d: %#llx\n", i, ((uint64_t*)MINERVA_OFFS()->symbolOffsets)[i]);
    }
}
