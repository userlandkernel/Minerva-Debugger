//
//  patches.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 7/28/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef patches_h
#define patches_h

#include <stdio.h>
#define KEXTLOGGING_MINERVA_DEFAULT 0xfff

#define DISABLE_KPTR_STRIPPING_MINERVA_DEFAULT 0x1

#define KDEBUG_MINERVA_DEFAULT KDEBUG_ENABLE_TRACE|KDEBUG_ENABLE_SERIAL

#define CSDEBUG_MINERVA_DEFAULT 0x10000000

//
#define IOKIT_DEBUG_MINERVA_DEFAULT kIOLogAttach|\
kIOLogCatalogue|kIOLogConfig|\
kIOLogDebugPower|kIOLogDTree|\
kIOLogHibernate|kIOLogKextMemory|\
kIOLogMemory|kIOLogMatch|\
kIOLogMapping|kIOLogPower|\
kIOLogProbe|kIOLogPMRootDomain|\
kIOLogRegister|kIOLogStart|\
kIOLogServiceTree|kIOLogTracePower|\
kIOLogYield
//

//
#define KERNEL_DEBUGFLAG_MINERVA_DEFAULT DB_NMI|\
DB_PRT_KDEBUG|\
DB_PRT|DB_KPRT|\
DB_ARP|DB_HALT|DB_KDP_BP_DIS|\
DB_LOG_PI_SCRN|DB_DBG_POST_CORE
//

void toggle_mac_vnode_enforce(void);

void set_serial_baudrate(uint32_t baudrate);
void set_kext_logging(uint32_t value);
void set_kdebug(uint32_t value);
void set_csdebug(uint32_t value);
void set_iokit_debug(uint64_t value);
void set_kernel_debugflag(uint32_t value);

void toggle_nvme_debugging(void);
void toggle_kext_assertions(void);
void toggle_kptr_stripping(void);
void toggle_panicdebugging(void);

#endif /* patches_h */
