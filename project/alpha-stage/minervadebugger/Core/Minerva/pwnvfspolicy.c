//
//  pwnvfspolicy.c
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/30/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include "pwnvfspolicy.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/event.h>
#include <sys/mount.h>
#include <sys/lock.h>
#include <sys/syscall.h>
#include "kutils.h"
#include "offsets.h"

#ifndef BOOL
#define BOOL bool
#endif

#include <CoreFoundation/CoreFoundation.h>
#include <jelbrek/jelbrek.h>

#undef vnode_t
#define VNODE_UPDATE_PARENT   0x01
#define VNODE_UPDATE_NAME     0x02
#define VNODE_UPDATE_CACHE    0x04
#define VNODE_UPDATE_PURGE    0x08

#define offsetof_v_name 0xd8-(sizeof(void*)+sizeof(void *)+sizeof(mount_t)+sizeof(void*))
LIST_HEAD(buflists, buf);

typedef struct {
    uintptr_t        opaque[2];
} lck_mtx_t;
typedef struct vnode {
    lck_mtx_t v_lock;            /* vnode mutex */
    TAILQ_ENTRY(vnode) v_freelist;        /* vnode freelist */
    TAILQ_ENTRY(vnode) v_mntvnodes;        /* vnodes for mount point */
    TAILQ_HEAD(, namecache) v_ncchildren;    /* name cache entries that regard us as their parent */
    LIST_HEAD(, namecache) v_nclinks;    /* name cache entries that name this vnode */
    vnode_t     v_defer_reclaimlist;        /* in case we have to defer the reclaim to avoid recursion */
    uint32_t v_listflag;            /* flags protected by the vnode_list_lock (see below) */
    uint32_t v_flag;            /* vnode flags (see below) */
    uint16_t v_lflag;            /* vnode local and named ref flags */
    uint8_t     v_iterblkflags;        /* buf iterator flags */
    uint8_t     v_references;            /* number of times io_count has been granted */
    int32_t     v_kusecount;            /* count of in-kernel refs */
    int32_t     v_usecount;            /* reference count of users */
    int32_t     v_iocount;            /* iocounters */
    void *   v_owner;            /* act that owns the vnode */
    uint16_t v_type;            /* vnode type */
    uint16_t v_tag;                /* type of underlying data */
    uint32_t v_id;                /* identity of vnode contents */
    union {
        struct mount    *vu_mountedhere;/* ptr to mounted vfs (VDIR) */
        struct socket    *vu_socket;    /* unix ipc (VSOCK) */
        struct specinfo    *vu_specinfo;    /* device (VCHR, VBLK) */
        struct fifoinfo    *vu_fifoinfo;    /* fifo (VFIFO) */
        struct ubc_info *vu_ubcinfo;    /* valid for (VREG) */
    } v_un;
    struct    buflists v_cleanblkhd;        /* clean blocklist head */
    struct    buflists v_dirtyblkhd;        /* dirty blocklist head */
    struct klist v_knotes;            /* knotes attached to this vnode */
    kauth_cred_t    v_cred;            /* last authorized credential */
    int    v_authorized_actions;    /* current authorized actions for v_cred */
    int        v_cred_timestamp;    /* determine if entry is stale for MNTK_AUTH_OPAQUE */
    int        v_nc_generation;    /* changes when nodes are removed from the name cache */
    int32_t        v_numoutput;            /* num of writes in progress */
    int32_t        v_writecount;            /* reference count of writers */
    const char *v_name;            /* name component of the vnode */
    vnode_t v_parent;            /* pointer to parent vnode */
    struct lockf    *v_lockf;        /* advisory lock list head */
    int     (**v_op)(void *);        /* vnode operations vector */
    mount_t v_mount;            /* ptr to vfs we are in */
    void *    v_data;                /* private data for fs */
    
} *vnode_t;

struct componentname {
    uint32_t    cn_nameiop;    /* lookup operation */
    uint32_t    cn_flags;    /* flags (see below) */
    
#ifdef BSD_KERNEL_PRIVATE
    vfs_context_t    cn_context;
    struct nameidata *cn_ndp;    /* pointer back to nameidata */
    
#define    cn_proc        (cn_context->vc_proc + 0)    /* non-lvalue */
#define    cn_cred        (cn_context->vc_ucred + 0)    /* non-lvalue */
    
#else
    void * cn_reserved1;    /* use vfs_context_t */
    void * cn_reserved2;    /* use vfs_context_t */
#endif
    char    *cn_pnbuf;    /* pathname buffer */
    int    cn_pnlen;    /* length of allocated buffer */
    char    *cn_nameptr;    /* pointer to looked up name */
    int    cn_namelen;    /* length of looked up component */
    uint32_t    cn_hash;    /* hash value of looked up name */
    uint32_t    cn_consume;    /* chars to consume in lookup() */
};

void cache_enter(mach_vm_address_t dvp, mach_vm_address_t vp, mach_vm_address_t cnp) {
    uint64_t cache_enter_sym = SYMOFF(_CACHE_ENTER);
    if(!cache_enter_sym){
        printf("Failed to find _cache_enter symbol.\n");
    }
    else {
        Kernel_Execute(cache_enter_sym, (uint64_t)dvp, (uint64_t)vp, (uint64_t)cnp, 0, 0, 0, 0);
    }
}

void vnode_update_identity(mach_vm_address_t vp, mach_vm_address_t dvp,  mach_vm_address_t name, int name_len, uint32_t name_hashval, int flags) {
    uint64_t vnode_update_identity_sym = SYMOFF(_VNODE_UPDATE_IDENTITY);
    if(!vnode_update_identity_sym){
        printf("Failed to find _vnode_update_identity symbol.\n");
    }
    else {
        Kernel_Execute(vnode_update_identity_sym, dvp, name, name_len, name_hashval, flags, 0, 0);
    }
}


kern_return_t pwnvfs_make_appleinternal(void){
    // Create fake /AppleInternals directory
    DIR* d = opendir("/AppleInternals");
    if(d){
        closedir(d);
    }
    if (ENOENT == errno)
    {
        minerva_info("Creating /AppleInternals directory so it can be patched...\n", NULL);
        int mkdir_ret = syscall(SYS_mkdir, "/AppleInternals", 0777, 0);
        minerva_info("Creation of directory: %s\n", mkdir_ret != -1 ? "Success." : "Failed");
        if(mkdir_ret == -1) {
            return KERN_FAILURE;
        }
    }
    
    // Lookup the virtualnode for the /AppleInternals directory on the filesystem.
    mach_vm_address_t vFakeInternal = 0;
    vnode_lookup("/AppleInternals", 0, &vFakeInternal, get_vfs_context());
    
    // In case the node is found copy it to userland
    if(vFakeInternal) {
        
        // Copy the name of the vnode to userland
        char* oldName = malloc(sizeof("AppleInternals"));
        mach_vm_address_t namePtr = (mach_vm_address_t)vFakeInternal+offsetof_v_name;
        copyin(oldName, ReadAnywhere64(namePtr), sizeof("AppleInternals"));
        
        minerva_info("Vnode name before patch: %s at %#llx\n", oldName, ReadAnywhere64(namePtr));
        
        // No more need to have the old name in userland
        free(oldName);
        oldName = NULL;
        
        // Create the new name in the kernel
        char newName[] = {'A','p','p','l','e','I','n','t','e','r','n','a','l', '\0'};
        mach_vm_address_t patchedNamePtr = ReadAnywhere64(namePtr);
        copyout(patchedNamePtr, newName, sizeof(newName));
        WriteAnywhere64(namePtr, patchedNamePtr);
        
        //Check
        char *patchedName = malloc(sizeof("AppleInternals"));
        copyin(patchedName, ReadAnywhere64(namePtr), sizeof("AppleInternals"));
        minerva_info("Vnode name after patch: %s at %#llx\n", patchedName, ReadAnywhere64(namePtr));
        
        // No more need to have the patched name in userland
        free(patchedName);
        patchedName = NULL;
        
        // Now we need to add it to the cache
        uint64_t rootVnode = 0;
        vnode_lookup("/", 0, &rootVnode, get_vfs_context());
        printf("Found / at: %#llx\n", rootVnode);
        
        // Now comes the hard part: updating it on the vfs cache
        struct componentname uCacheCompInternal = {};
        uCacheCompInternal.cn_nameptr = (void*)patchedNamePtr;
        uCacheCompInternal.cn_namelen = sizeof("AppleInternal");
        
        mach_vm_address_t kCacheCompInternal = Kernel_alloc(sizeof(struct componentname));
        copyout(kCacheCompInternal, &uCacheCompInternal, sizeof(sizeof(struct componentname)));
        
        printf("And now the real stuff happens...\n");
        sleep(3);
        vnode_update_identity(vFakeInternal, rootVnode, (mach_vm_address_t)uCacheCompInternal.cn_nameptr, 0, 0, VNODE_UPDATE_PARENT|VNODE_UPDATE_NAME|VNODE_UPDATE_CACHE|VNODE_UPDATE_PURGE);
        vnode_put(vFakeInternal);
        
        // Try to look it up :)
        uint64_t realInternalVnode = 0;
        vnode_lookup("AppleInternal", 0, &realInternalVnode, get_vfs_context());
        
        printf("Real vnode is now: %#llx\n", realInternalVnode);
        return KERN_SUCCESS;
        
    }
    return KERN_FAILURE;
}
/*
void vnode_bypass_mkdir(const char *realpath){
    // Create fake /AppleInternals directory
    char fakepath[strlen(realpath)+1]
    strcpy(fakepath, realpath);
    fakepath[strlen(realpath)] = 'F';
    DIR* d = opendir(fakepath);
    if(d){
        closedir(d);
    }
    if (ENOENT == errno)
    {
        minerva_info("Creating /AppleInternals directory so it can be patched...\n", NULL);
        int mkdir_ret = syscall(SYS_mkdir, fakepath, 0777, 0);
        minerva_info("Creation of directory: %s\n", mkdir_ret != -1 ? "Success." : "Failed");
        if(mkdir_ret == -1) {
            return KERN_FAILURE;
        }
    }
    
    // Lookup the virtualnode for the /AppleInternals directory on the filesystem.
    mach_vm_address_t vFakeInternal = 0;
    vnode_lookup("/AppleInternals", 0, &vFakeInternal, get_vfs_context());
    
    // In case the node is found copy it to userland
    if(vFakeInternal) {
        
        // Copy the name of the vnode to userland
        char* oldName = malloc(sizeof("AppleInternals"));
        mach_vm_address_t namePtr = (mach_vm_address_t)vFakeInternal+offsetof_v_name;
        copyin(oldName, ReadAnywhere64(namePtr), sizeof("AppleInternals"));
        
        minerva_info("Vnode name before patch: %s at %#llx\n", oldName, ReadAnywhere64(namePtr));
        
        // No more need to have the old name in userland
        free(oldName);
        oldName = NULL;
        
        // Create the new name in the kernel
        char newName[] = {'A','p','p','l','e','I','n','t','e','r','n','a','l', '\0'};
        mach_vm_address_t patchedNamePtr = ReadAnywhere64(namePtr);
        copyout(patchedNamePtr, newName, sizeof(newName));
        WriteAnywhere64(namePtr, patchedNamePtr);
        
        //Check
        char *patchedName = malloc(sizeof("AppleInternals"));
        copyin(patchedName, ReadAnywhere64(namePtr), sizeof("AppleInternals"));
        minerva_info("Vnode name after patch: %s at %#llx\n", patchedName, ReadAnywhere64(namePtr));
        
        // No more need to have the patched name in userland
        free(patchedName);
        patchedName = NULL;
        
        // Now we need to add it to the cache
        uint64_t rootVnode = 0;
        vnode_lookup("/", 0, &rootVnode, get_vfs_context());
        printf("Found / at: %#llx\n", rootVnode);
        
        // Now comes the hard part: updating it on the vfs cache
        struct componentname uCacheCompInternal = {};
        uCacheCompInternal.cn_nameptr = (void*)patchedNamePtr;
        uCacheCompInternal.cn_namelen = sizeof("AppleInternal");
        
        mach_vm_address_t kCacheCompInternal = Kernel_alloc(sizeof(struct componentname));
        copyout(kCacheCompInternal, &uCacheCompInternal, sizeof(sizeof(struct componentname)));
        
        printf("And now the real stuff happens...\n");
        sleep(3);
        vnode_update_identity(vFakeInternal, rootVnode, (mach_vm_address_t)uCacheCompInternal.cn_nameptr, 0, 0, VNODE_UPDATE_PARENT|VNODE_UPDATE_NAME|VNODE_UPDATE_CACHE|VNODE_UPDATE_PURGE);
        vnode_put(vFakeInternal);
        
        // Try to look it up :)
        uint64_t realInternalVnode = 0;
        vnode_lookup("AppleInternal", 0, &realInternalVnode, get_vfs_context());
        
        printf("Real vnode is now: %#llx\n", realInternalVnode);
        
    }
}*/
