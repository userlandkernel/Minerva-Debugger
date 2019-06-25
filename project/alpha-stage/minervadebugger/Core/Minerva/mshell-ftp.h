//
//  mshell-ftp.h
//  minervadebugger
//
//  Created by Sem Voigtländer on 6/25/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef mshell_ftp_h
#define mshell_ftp_h

#include <stdio.h>
int ftp_authentication(const char *account,  const char *password, void *user_data);
#endif /* mshell_ftp_h */
