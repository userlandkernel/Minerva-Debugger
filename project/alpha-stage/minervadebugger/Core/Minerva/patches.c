//
//  patches.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 7/28/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//
#include <stdlib.h>
#include "patches.h"
#include "offsets.h"
#include "serial.h"
#include "kutils.h"
#include "xnu-header.h"

void toggle_mac_vnode_enforce(void){
    if(ReadAnywhereBool(SLIDADDR(0xFFFFFFF0075FF710))){
        SUPER_PRINTF("Loosening mac vnode_enforce policy...\n", NULL);
        WriteAnywhereBool(SLIDADDR(0xFFFFFFF0075FF710), FALSE);
    }
    else {
        SUPER_PRINTF("Hardening mac vnode enforce policy...\n", NULL);
        WriteAnywhereBool(SLIDADDR(0xFFFFFFF0075FF710), TRUE);
    }
}

void set_serial_baudrate(uint32_t baudrate){
    if(baudrate == ReadAnywhere32(PATCHOFF(GPE_SERIAL_BAUD))){
        return; // No need to patch again
    }
    SUPER_PRINTF("Setting baud rate to %d...\n", baudrate);
    WriteAnywhere32(PATCHOFF(GPE_SERIAL_BAUD), baudrate); // Set baudrate
}

/*
 Unknown debug:
 
 WriteAnywhere64(0xFFFFFFF007095C40+slide, 0x3); // 1 | 2, according to IDA (assertions etc.).
 
 --> Sadly is in KTRR region

 */

void toggle_nvme_debugging(void){
    if(ReadAnywhere8(PATCHOFF(GNVME_DEBUGFLAGS)) == TRUE){
        SUPER_PRINTF("Disabling GNVME debugging...\n", NULL);
    }
    else {
        SUPER_PRINTF("Enabling GNVME debugging...\n", NULL);
        WriteAnywhere32(PATCHOFF(GNVME_DEBUGFLAGS), TRUE);
    }
}

void set_kext_logging(uint32_t value){
    uint32_t old = ReadAnywhere32(SLIDADDR(0xFFFFFFF007763950));
    if(old == value){
        return; // No need to update
    }
    SUPER_PRINTF("Setting KEXT Logging level from %#x to %#x...\n", old, value);
    WriteAnywhere32(SLIDADDR(0xFFFFFFF007763950), value);
}

void toggle_kext_assertions(void){
    if(ReadAnywhere8(SLIDADDR(0xFFFFFFF00760ED24))) {
        SUPER_PRINTF("Disabling KEXT assertions...\n", NULL);
        WriteAnywhere32(SLIDADDR(0xFFFFFFF007602030), FALSE);
    }
    else {
        printf("Enabling KEXT assertions...\n");
        WriteAnywhere32(SLIDADDR(0xFFFFFFF007602030), TRUE);
    }
}

void toggle_kptr_stripping(void){
    if(ReadAnywhere8(SLIDADDR(0xFFFFFFF00760ED24))) {
        SUPER_PRINTF("Enabling kernel pointer stripping...\n", NULL);
        WriteAnywhereBool(SLIDADDR(0xFFFFFFF00760ED24), FALSE);
    }
    else {
        SUPER_PRINTF("Disabling kernel pointer stripping ...\n", NULL);
        WriteAnywhereBool(SLIDADDR(0xFFFFFFF00760ED24), TRUE);
    }
}
void set_kdebug(uint32_t value){
    uint32_t old = ReadAnywhere32(SYMOFF(_KDEBUG_ENABLE));
    if(old == value){
        return; // No need to update
    }
    SUPER_PRINTF("Setting kdebug from %#x to %#x...\n", old, value);
    WriteAnywhere32(SYMOFF(_KDEBUG_ENABLE), value); // Enable all debugging
}

void set_csdebug(uint32_t value){
    uint32_t old = ReadAnywhere32(SYMOFF(_CS_DEBUG));
    if(old == value){
        return; // No need to update
    }
    SUPER_PRINTF("Setting codesignature debugging from %#x to %#x...\n", old, value);
    WriteAnywhere32(SYMOFF(_CS_DEBUG), value); // Enable codesigning debugging
}

void toggle_panicdebugging(void){
    if(ReadAnywhereBool(PATCHOFF(ENABLE_PANICDBG))){
        SUPER_PRINTF("Disabling panic debugging...\n", NULL);
        WriteAnywhereBool(PATCHOFF(ENABLE_PANICDBG), FALSE); // Enable panic debugging
    } else {
        SUPER_PRINTF("Enabling panic debugging...\n", NULL);
        WriteAnywhereBool(PATCHOFF(ENABLE_PANICDBG), TRUE); // Enable panic debugging
    }
}

void set_iokit_debug(uint64_t value){
    uint64_t old = ReadAnywhere64(SYMOFF(_GIOKITDEBUG));
    if(old == value){
        return; // No need to update
    }
    SUPER_PRINTF("Setting kdebug from %#llx to %#llx...\n", old, value);
    WriteAnywhere64(SYMOFF(_GIOKITDEBUG), value); // Enable IOKit debugging
}

void set_kernel_debugflag(uint32_t value){
    uint32_t old = ReadAnywhere32(SYMOFF(_DEBUGFLAG));
    if(old == value){
        return; // No need to update
    }
    SUPER_PRINTF("Setting kernel debugflag from %#x to %#x...\n", old, value);
    WriteAnywhere32(SYMOFF(_DEBUGFLAG), value); // Enable lots of kernel debugging
}
