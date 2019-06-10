//
//  init.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/4/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef init_h
#define init_h

#include <stdio.h>
#include "offsets.h"
enum minerva_os_stage {
    STAGE_DEV = 0x20,
    STAGE_ALPHA = 0x40,
    STAGE_BETA = 0x60,
    STAGE_RELEASE = 0x80
};

struct minerva_utsname {
    char *sysname; /* [XSI] Name of OS */
    char *nodename; /* [XSI] Name of this network node */
    char *release; /* [XSI] Release level */
    char *version; /* [XSI] Version level */
    char *machine; /* [XSI] Hardware type */
};

#define MINERVA_VERSION_MAJOR 1
#define MINERVA_VERSION_MINOR 0
#define MINERVA_REVISION 1


#define MINERVA_STAGE STAGE_DEV
#define MINERVA_PRERELEASE_LEVEL 0

#define MINERVA_VERSION_VARIANT "1"

#define MINERVA_OSBUILDER "root"
#define MINERVA_OSRELEASE "ukern-100~0/DEV_ARM64"
#define MINERVA_OSTYPE "MinervaCore"

#endif /* init_h */
