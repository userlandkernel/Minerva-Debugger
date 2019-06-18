//
//  SYS_ARM64.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "defeat.h"
#include <mach/mach.h>
#define BOOL bool
#include <stdbool.h>
#include <jelbrek/jelbrek.h>
#include "kutils.h"
#include <stdlib.h>

uint64_t tcr_tg0_granule_size = 0;

void set_tcr(uint64_t bit){
    uint64_t new_tcr = 0;
    new_tcr = (TCR_TG1_GRANULE_64KB << TCR_T1SZ_SHIFT) & TCR_TSZ_MASK;
    Kernel_Execute(0xFFFFFFF0070A797+slide, new_tcr, 0, 0, 0, 0, 0, 0);
}


