//
//  AppDelegate.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/5/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreData/CoreData.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@property (readonly, strong) NSPersistentContainer *persistentContainer;

- (void)saveContext;


@end

