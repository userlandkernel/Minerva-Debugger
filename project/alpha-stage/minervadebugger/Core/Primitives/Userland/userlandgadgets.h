//
//  dxnu_gadgets.h
//  pwn
//
//  Created by Sem Voigtländer on 1/8/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef dxnu_gadgets_h
#define dxnu_gadgets_h

#include <stdio.h>
uint64_t find_userlandgadget_candidate(char** alternatives, size_t gadget_length);
#endif /* dxnu_gadgets_h */
