//
//  devicetree.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef devicetree_h
#define devicetree_h

#include <stdio.h>
#include <mach/mach.h>
enum {
    kDTPathNameSeparator = '/' /* 0x2F */
};
enum {
    kDTMaxPropertyNameLength=31
};

typedef char DTPropertyNameBuf[32];

enum {
    kDTMaxEntryNameLength = 63
};

typedef char DTEntryNameBuf[kDTMaxEntryNameLength+1];

#define kPropNameLength 32

typedef struct DeviceTreeNodeProperty {
    char        name[kPropNameLength];
    uint32_t        length;
} DeviceTreeNodeProperty;

typedef struct OpaqueDTEntry {
    uint32_t        nProperties;    // Number of props[] elements (0 => end)
    uint32_t        nChildren;    // Number of children[] elements
} DeviceTreeNode;

typedef DeviceTreeNode *RealDTEntry;

typedef struct DTSavedScope {
    struct DTSavedScope * nextScope;
    RealDTEntry scope;
    RealDTEntry entry;
    unsigned long index;
} *DTSavedScopePtr;

/* Entry Iterator*/
typedef struct OpaqueDTEntryIterator {
    RealDTEntry outerScope;
    RealDTEntry currentScope;
    RealDTEntry currentEntry;
    DTSavedScopePtr savedScope;
    unsigned long currentIndex;
} OpaqueDTEntryIterator, *DTEntryIterator;

/* Property Iterator*/
typedef struct OpaqueDTPropertyIterator {
    RealDTEntry entry;
    DeviceTreeNodeProperty *currentProperty;
    unsigned long currentIndex;
} OpaqueDTPropertyIterator, *DTPropertyIterator;

/* Entry*/
typedef struct OpaqueDTEntry* DTEntry;

/* Entry Iterator*/
typedef struct OpaqueDTEntryIterator* DTEntryIterator;

/* Property Iterator*/
typedef struct OpaqueDTPropertyIterator* DTPropertyIterator;

enum {
    kError = -1,
    kIterationDone = 0,
    kSuccess = 1
};
int DTFindEntry(const char *propName, const char *propValue, mach_vm_address_t *entryH);
kern_return_t DTLookupEntry(mach_vm_address_t num, const char* name, mach_vm_address_t* entry);
kern_return_t DTGetProperty(mach_vm_address_t entry, const char* name, mach_vm_address_t* reg_prop, mach_vm_size_t* size);
#endif /* devicetree_h */
