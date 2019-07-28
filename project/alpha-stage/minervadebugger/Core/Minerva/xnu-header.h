//
//  Header.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/4/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef xnu_header_h
#define xnu_header_h
#include <mach/mach.h>
#include <stdbool.h>
#define MAXPRI_KERNEL 95

#define DBG_MACH_EXCP_KTRAP_x86 0x02 /* Kernel Traps on x86 */
#define DBG_MACH_EXCP_DFLT      0x03 /* Data Translation Fault */
#define DBG_MACH_EXCP_IFLT      0x04 /* Inst Translation Fault */
#define DBG_MACH_EXCP_INTR      0x05 /* Interrupts */
#define DBG_MACH_EXCP_ALNG      0x06 /* Alignment Exception */
#define DBG_MACH_EXCP_UTRAP_x86 0x07 /* User Traps on x86 */
#define DBG_MACH_EXCP_FP        0x08 /* FP Unavail */
#define DBG_MACH_EXCP_DECI      0x09 /* Decrementer Interrupt */
#define DBG_MACH_CHUD           0x0A /* deprecated name */
#define DBG_MACH_SIGNPOST       0x0A /* kernel signposts */
#define DBG_MACH_EXCP_SC        0x0C /* System Calls */
#define DBG_MACH_EXCP_TRACE     0x0D /* Trace exception */
#define DBG_MACH_EXCP_EMUL      0x0E /* Instruction emulated */
#define DBG_MACH_IHDLR          0x10 /* Interrupt Handlers */
#define DBG_MACH_IPC            0x20 /* Inter Process Comm */
#define DBG_MACH_RESOURCE       0x25 /* tracing limits, etc */
#define DBG_MACH_VM             0x30 /* Virtual Memory */
#define DBG_MACH_LEAKS          0x31 /* alloc/free */
#define DBG_MACH_WORKINGSET     0x32 /* private subclass for working set related debugging */
#define DBG_MACH_SCHED          0x40 /* Scheduler */
#define DBG_MACH_MSGID_INVALID  0x50 /* Messages - invalid */
#define DBG_MACH_LOCKS          0x60 /* new lock APIs */
#define DBG_MACH_PMAP           0x70 /* pmap */
#define DBG_MACH_CLOCK          0x80 /* clock */
#define DBG_MACH_MP             0x90 /* MP related */
#define DBG_MACH_VM_PRESSURE    0xA0 /* Memory Pressure Events */
#define DBG_MACH_STACKSHOT      0xA1 /* Stackshot/Microstackshot subsystem */
#define DBG_MACH_SFI            0xA2 /* Selective Forced Idle (SFI) */
#define DBG_MACH_ENERGY_PERF    0xA3 /* Energy/performance resource stats */
#define DBG_MACH_SYSDIAGNOSE    0xA4 /* sysdiagnose */
#define DBG_MACH_ZALLOC         0xA5 /* Zone allocator */
#define DBG_MACH_THREAD_GROUP   0xA6 /* Thread groups */
#define DBG_MACH_COALITION      0xA7 /* Coalitions */
#define DBG_MACH_SHAREDREGION   0xA8 /* Shared region */

#define KDEBUG_ENABLE_TRACE   (1U << 0)
#define KDEBUG_ENABLE_ENTROPY (1U << 1) /* obsolete */
#define KDEBUG_ENABLE_CHUD    (1U << 2) /* obsolete */
#define KDEBUG_ENABLE_PPT     (1U << 3)
#define KDEBUG_ENABLE_SERIAL  (1U << 4)

#define kPEGraphicsMode        1
#define kPETextMode        2
#define kPETextScreen        3
#define kPEAcquireScreen    4
#define kPEReleaseScreen    5
#define kPEEnableScreen         6
#define kPEDisableScreen    7
#define kPEBaseAddressChange    8
#define kPERefreshBootGraphics    9

#define NO_CUR_DB       0x0
#define KDP_CUR_DB      0x1

#define DEBUGGER_OPTION_NONE                        0x0ULL
#define DEBUGGER_OPTION_PANICLOGANDREBOOT           0x1ULL
#define DEBUGGER_OPTION_RECURPANIC_ENTRY            0x2ULL
#define DEBUGGER_OPTION_RECURPANIC_PRELOG           0x4ULL
#define DEBUGGER_OPTION_RECURPANIC_POSTLOG          0x8ULL
#define DEBUGGER_OPTION_RECURPANIC_POSTCORE         0x10ULL
#define DEBUGGER_OPTION_INITPROC_PANIC              0x20ULL
#define DEBUGGER_OPTION_COPROC_INITIATED_PANIC      0x40ULL
#define DEBUGGER_OPTION_SKIP_LOCAL_COREDUMP         0x80ULL
#define DEBUGGER_OPTION_ATTEMPTCOREDUMPANDREBOOT    0x100ULL
#define DEBUGGER_INTERNAL_OPTION_THREAD_BACKTRACE   0x200ULL

#define DEBUGGER_INTERNAL_OPTIONS_MASK              (DEBUGGER_INTERNAL_OPTION_THREAD_BACKTRACE)

#define DB_HALT                     0x1
#define DB_PRT                      0x2
#define DB_NMI                      0x4
#define DB_KPRT                     0x8
#define DB_KDB                      0x10
#define DB_ARP                      0x40
#define DB_KDP_BP_DIS               0x80
#define DB_LOG_PI_SCRN              0x100
#define DB_KDP_GETC_ENA             0x200
#define DB_KERN_DUMP_ON_PANIC       0x400
#define DB_KERN_DUMP_ON_NMI         0x800
#define DB_DBG_POST_CORE            0x1000
#define DB_PANICLOG_DUMP            0x2000
#define DB_REBOOT_POST_CORE         0x4000
#define DB_NMI_BTN_ENA              0x8000
#define DB_PRT_KDEBUG               0x10000
#define DB_DISABLE_LOCAL_CORE       0x20000
#define DB_DISABLE_GZIP_CORE        0x40000
#define DB_DISABLE_CROSS_PANIC      0x80000
#define DB_REBOOT_ALWAYS            0x100000

struct vc_progress_element
{
    unsigned int version;
    unsigned int flags;
    unsigned int time;
    uint8_t count;
    uint8_t res[3];
    int width;
    int height;
    int dx;
    int dy;
    int transparent;
    unsigned int res2[3];
};


struct PE_Video
{
    uint64_t v_baseAddr;
    uint64_t v_rowBytes;
    uint64_t v_width;
    uint64_t v_height;
    uint64_t v_depth;
    uint64_t v_display;
    char v_pixelFormat[64];
    uint64_t  v_offset;
    uint64_t  v_length;
    bool v_rotate;
    uint64_t v_scale;
    char reserved1[2];
    int64_t reserved2;
};

typedef uint64_t dbgwrap_reg_t;
typedef arm_thread_state64_t __attribute__((aligned(16))) dbgwrap_thread_state_t;
typedef enum {
    DBGWRAP_ERR_SELF_HALT = -6,
    DBGWRAP_ERR_UNSUPPORTED = -5,
    DBGWRAP_ERR_INPROGRESS = -4,
    DBGWRAP_ERR_INSTR_ERROR = -3,
    DBGWRAP_ERR_INSTR_TIMEOUT = -2,
    DBGWRAP_ERR_HALT_TIMEOUT = -1,
    DBGWRAP_SUCCESS = 0,
    DBGWRAP_WARN_ALREADY_HALTED,
    DBGWRAP_WARN_CPU_OFFLINE
} dbgwrap_status_t;


enum {
    // loggage
    kIOLogAttach        =         0x00000001ULL,
    kIOLogProbe         =         0x00000002ULL,
    kIOLogStart         =         0x00000004ULL,
    kIOLogRegister      =         0x00000008ULL,
    kIOLogMatch         =         0x00000010ULL,
    kIOLogConfig        =         0x00000020ULL,
    kIOLogYield         =         0x00000040ULL,
    kIOLogPower         =         0x00000080ULL,
    kIOLogMapping       =         0x00000100ULL,
    kIOLogCatalogue     =         0x00000200ULL,
    kIOLogTracePower    =         0x00000400ULL,  // Obsolete: Use iotrace=0x00000400ULL to enable now
    kIOLogDebugPower    =         0x00000800ULL,
    kIOLogServiceTree   =         0x00001000ULL,
    kIOLogDTree         =         0x00002000ULL,
    kIOLogMemory        =         0x00004000ULL,
    kIOLogKextMemory    =         0x00008000ULL,
    kOSLogRegistryMods  =         0x00010000ULL,  // Log attempts to modify registry collections
    kIOLogPMRootDomain  =         0x00020000ULL,
    kOSRegistryModsMode =         0x00040000ULL,  // Change default registry modification handling - panic vs. log
    //    kIOTraceIOService   =         0x00080000ULL,  // Obsolete: Use iotrace=0x00080000ULL to enable now
    kIOLogHibernate     =         0x00100000ULL,
    kIOStatistics       =         0x04000000ULL,
    kIOSleepWakeWdogOff =         0x40000000ULL,
    kIOKextSpinDump     =         0x80000000ULL,
    
    // debug aids - change behaviour
    kIONoFreeObjects    =         0x00100000ULL,
    //    kIOLogSynchronous   =         0x00200000ULL,  // IOLog completes synchronously -- obsolete
    kIOTracking         =         0x00400000ULL,
    kIOWaitQuietPanics  =         0x00800000ULL,
    kIOWaitQuietBeforeRoot =      0x01000000ULL,
    kIOTrackingBoot     =         0x02000000ULL,
    
    _kIODebugTopFlag    = 0x8000000000000000ULL   // force enum to be 64 bits
};

enum {
    kIOKitDebugUserOptions = 0
    | kIOLogAttach
    | kIOLogProbe
    | kIOLogStart
    | kIOLogRegister
    | kIOLogMatch
    | kIOLogConfig
    | kIOLogYield
    | kIOLogPower
    | kIOLogMapping
    | kIOLogCatalogue
    | kIOLogTracePower
    | kIOLogDebugPower
    | kOSLogRegistryMods
    | kIOLogPMRootDomain
    | kOSRegistryModsMode
    | kIOLogHibernate
    | kIOSleepWakeWdogOff
    | kIOKextSpinDump
    | kIOWaitQuietPanics
};

enum {
    kIOTraceInterrupts        =         0x00000001ULL,    // Trace primary interrupts
    kIOTraceWorkLoops        =        0x00000002ULL,    // Trace workloop activity
    kIOTraceEventSources    =        0x00000004ULL,    // Trace non-passive event sources
    kIOTraceIntEventSource    =        0x00000008ULL,    // Trace IOIES and IOFIES sources
    kIOTraceCommandGates    =        0x00000010ULL,    // Trace command gate activity
    kIOTraceTimers            =         0x00000020ULL,    // Trace timer event source activity
    
    kIOTracePowerMgmt        =        0x00000400ULL,    // Trace power management changes
    
    kIOTraceIOService       =        0x00080000ULL,    // registerService/termination
    
    kIOTraceCompatBootArgs    =        kIOTraceIOService | kIOTracePowerMgmt
};

// Locking
#define    simple_lock_addr(lock)    (&(lock))
struct slock {
    volatile natural_t lock_data;    /* in general 1 bit is sufficient */
};

typedef struct slock    simple_lock_data_t;
#define    decl_simple_lock_data(class,name) \
class    simple_lock_data_t    name;

#endif /* Header_h */
