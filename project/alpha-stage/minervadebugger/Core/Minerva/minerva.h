//
//  minerva.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 5/5/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef minerva_h
#define minerva_h

#include <stdio.h>
#include <mach/mach.h>
#include <mach/machvm.h>
#ifndef BOOL
#define BOOL bool
#endif
kern_return_t minerva_init(void);
#endif /* minerva_h */
