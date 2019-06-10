
//
//  sbsuspend.m
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/7/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <dlfcn.h>
#import "sbsuspend.h"

#define SBS_PATH "/System/Library/PrivateFrameworks/SpringBoardServices.framework/SpringBoardServices"

@implementation SpringBoardServicesClient

+ (void)initialize {
    [super initialize];
    SpringBoardServicesHandle = dlopen(SBS_PATH, RTLD_LAZY);
    NSParameterAssert(SpringBoardServicesHandle);
}

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken = 0;
    static SpringBoardServicesClient *sharedSpringBoardServicesClient = nil;
    
    dispatch_once(&onceToken, ^{
        sharedSpringBoardServicesClient = [[self alloc] init];
    });
    
    return sharedSpringBoardServicesClient;
}

+ (mach_port_t)mainSpringBoardServicesMachPort {
    mach_port_t (*SBSSpringBoardServerPort)() = dlsym(SpringBoardServicesHandle, "SBSSpringBoardServerPort");
    NSParameterAssert(SBSSpringBoardServerPort);
    return SBSSpringBoardServerPort();
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _sbsMachPort = [self.class mainSpringBoardServicesMachPort];
    }
    return self;
}

- (void)dealloc {
    dlclose(SpringBoardServicesHandle);
}

- (void)callSBGetScreenLockStatusWithLocked:(BOOL*)isLocked andPasscodeEnabled:(BOOL*)passcodeEnabled{
    void *(*SBGetScreenLockStatus)(mach_port_t port, BOOL *isLocked, BOOL *passcodeEnabled) = dlsym(SpringBoardServicesHandle, "SBGetScreenLockStatus");
    NSParameterAssert(SBGetScreenLockStatus);
    SBGetScreenLockStatus(self.sbsMachPort, isLocked, passcodeEnabled);
}

- (BOOL)SBGetScreenLockStatusIsLocked{
    BOOL isLocked, passcodeEnabled;
    [self callSBGetScreenLockStatusWithLocked:&isLocked andPasscodeEnabled:&passcodeEnabled];
    return isLocked;
}

- (BOOL)SBGetScreenLockStatusIsPasscodeEnabled{
    BOOL isLocked, passcodeEnabled;
    [self callSBGetScreenLockStatusWithLocked:&isLocked andPasscodeEnabled:&passcodeEnabled];
    return passcodeEnabled;
}

- (void)SBSuspend{
    SpringBoardServicesReturn (*SBSuspend)(mach_port_t port) = dlsym(SpringBoardServicesHandle, "SBSuspend");
    NSParameterAssert(SBSuspend);
    SBSuspend(self.sbsMachPort);
}

- (void)SBSUndimScreen{
    void *(*SBSUndimScreen)() = dlsym(SpringBoardServicesHandle, "SBSUndimScreen");
    NSParameterAssert(SBSUndimScreen);
    SBSUndimScreen();
}

@end
