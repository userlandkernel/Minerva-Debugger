//
//  SYS_ARM64.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef SYS_ARM64_h
#define SYS_ARM64_h

#include <stdio.h>

#define TCR_T0SZ_SHIFT                0ULL
#define TCR_TSZ_BITS                6ULL
#define TCR_TSZ_MASK                ((1ULL << TCR_TSZ_BITS) - 1ULL)
#define TCR_IRGN0_SHIFT                8ULL
#define TCR_IRGN0_DISABLED            (0ULL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITEBACK            (1ULL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITETHRU            (2ULL << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WRITEBACKNO        (3ULL << TCR_IRGN0_SHIFT)
#define TCR_ORGN0_SHIFT                10ULL
#define TCR_ORGN0_DISABLED            (0ULL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITEBACK            (1ULL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITETHRU            (2ULL << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WRITEBACKNO        (3ULL << TCR_ORGN0_SHIFT)
#define TCR_SH0_SHIFT                12ULL
#define TCR_SH0_NONE                (0ULL << TCR_SH0_SHIFT)
#define TCR_SH0_OUTER                (2ULL << TCR_SH0_SHIFT)
#define TCR_SH0_INNER                (3ULL << TCR_SH0_SHIFT)
#define TCR_TG0_GRANULE_SHIFT        (14ULL)
#define TCR_TG0_GRANULE_4KB            (0ULL << TCR_TG0_GRANULE_SHIFT)
#define TCR_TG0_GRANULE_64KB        (1ULL << TCR_TG0_GRANULE_SHIFT)
#define TCR_TG0_GRANULE_16KB        (2ULL << TCR_TG0_GRANULE_SHIFT)

#if __ARM_16K_PG__
#define TCR_TG0_GRANULE_SIZE        (TCR_TG0_GRANULE_16KB)
#else
#define TCR_TG0_GRANULE_SIZE        (TCR_TG0_GRANULE_4KB)
#endif

#define TCR_T1SZ_SHIFT                16ULL
#define TCR_A1_ASID1                (1ULL << 22ULL)
#define TCR_EPD1_TTBR1_DISABLED        (1ULL << 23ULL)
#define TCR_IRGN1_SHIFT                24ULL
#define TCR_IRGN1_DISABLED            (0ULL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITEBACK            (1ULL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITETHRU            (2ULL << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WRITEBACKNO        (3ULL << TCR_IRGN1_SHIFT)
#define TCR_ORGN1_SHIFT                26ULL
#define TCR_ORGN1_DISABLED            (0ULL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITEBACK            (1ULL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITETHRU            (2ULL << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WRITEBACKNO        (3ULL << TCR_ORGN1_SHIFT)
#define TCR_SH1_SHIFT                28ULL
#define TCR_SH1_NONE                (0ULL << TCR_SH1_SHIFT)
#define TCR_SH1_OUTER                (2ULL << TCR_SH1_SHIFT)
#define TCR_SH1_INNER                (3ULL << TCR_SH1_SHIFT)
#define TCR_TG1_GRANULE_SHIFT        30ULL
#define TCR_TG1_GRANULE_16KB        (1ULL << TCR_TG1_GRANULE_SHIFT)
#define TCR_TG1_GRANULE_4KB            (2ULL << TCR_TG1_GRANULE_SHIFT)
#define TCR_TG1_GRANULE_64KB        (3ULL << TCR_TG1_GRANULE_SHIFT)
#if __ARM_16K_PG__
#define TCR_TG1_GRANULE_SIZE        (TCR_TG1_GRANULE_16KB)
#else
#define TCR_TG1_GRANULE_SIZE        (TCR_TG1_GRANULE_4KB)
#endif
#define TCR_IPS_SHIFT                32ULL
#define TCR_IPS_32BITS                (0ULL << TCR_IPS_SHIFT)
#define TCR_IPS_36BITS                (1ULL << TCR_IPS_SHIFT)
#define TCR_IPS_40BITS                (2ULL << TCR_IPS_SHIFT)
#define TCR_IPS_42BITS                (3ULL << TCR_IPS_SHIFT)
#define TCR_IPS_44BITS                (4ULL << TCR_IPS_SHIFT)
#define TCR_IPS_48BITS                (5ULL << TCR_IPS_SHIFT)
#define TCR_AS_16BIT_ASID            (1ULL << 36)
#define TCR_TBI0_TOPBYTE_IGNORED    (1ULL << 37)
#define TCR_TBI1_TOPBYTE_IGNORED    (1ULL << 38)

#define ASID_SHIFT            (12)                /* Shift for the maximum virtual ASID value (2048)*/
#define MAX_ASID            (1 << ASID_SHIFT)        /* Max supported ASIDs (can be virtual) */
#define ARM_ASID_SHIFT            (8)                /* Shift for the maximum ARM ASID value (256) */
#define ARM_MAX_ASID            (1 << ARM_ASID_SHIFT)        /* Max ASIDs supported by the hardware */
#define ASID_VIRT_BITS            (ASID_SHIFT - ARM_ASID_SHIFT)    /* The number of virtual bits in a virtaul ASID */

#define BOOTSTRAP_TABLE_SIZE (ARM_PGBYTES * 8)
#define PGTABLE_ADDR_BITS (64ULL - T0SZ_BOOT)
void set_tcr(uint64_t bit);
#endif /* SYS_ARM64_h */
