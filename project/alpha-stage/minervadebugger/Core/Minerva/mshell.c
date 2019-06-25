//
//  not_kdpshell.c
//  NotKDPShell
//
//  Created by Sem Voigtländer on 3/1/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include <mach/mach.h>
#include "mshell_cmd_bundle.h"
#include "mshell.h"
#include "simpleserver.h"
#include "ANSIColors.h"

kern_return_t run_mshell(const int port, const char *ip){
    kern_return_t err = KERN_SUCCESS;
    printf("Creating new NotKDPServer on port %s:%d.\n", ip, port);
    simpleserver* kdp_server = simple_server_create(port, ip, "minerva_server"); //we allocate a new server
    // Retrieve the human-readable UUID of the server
    char uuid[37] = {};
    uuid_unparse(kdp_server->uniqueId, uuid);
    printf("Starting the NotKDPServer with id: %s\n", uuid);
    simple_server_start(kdp_server); //we start listening on the server
    /// SOCKET_LOOP_START
    printf("Assigning a connection handler to the server. This will be our KDPShell commandline.\n");
    simple_connection_handler(kdp_server, current_interpreter, ANSI_COLOR_BLUE "root" ANSI_COLOR_RESET "@" ANSI_COLOR_RED "xnu" ANSI_COLOR_RESET "# "); //we allocate a listener for receiving from clients
    /// SOCKET_LOOP_END
    
    printf("Stopping server...\n");
    simple_server_stop(kdp_server); //we end the server if the client invoked a shutdown request
    return err;
}
