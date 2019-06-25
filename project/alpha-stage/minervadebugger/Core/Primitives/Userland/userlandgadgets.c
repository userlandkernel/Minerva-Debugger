//
//  dxnu_gadgets.c
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "userlandgadgets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
uint64_t
find_userlandgadget_candidate(char** alternatives, size_t gadget_length)
{
    void* haystack_start = (void*)atoi;
    size_t haystack_size = 100*1024*1024;
    
    for (char* candidate = *alternatives; candidate != NULL; alternatives++) {
        void* found_at = memmem(haystack_start, haystack_size, candidate, gadget_length);
        if (found_at != NULL){
            printf("found at: %llx\n", (uint64_t)found_at);
            return (uint64_t)found_at;
        }
    }
    
    return 0;
}
