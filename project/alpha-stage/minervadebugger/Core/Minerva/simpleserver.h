//
//  simpleserver.h
//  NotKDPShell
//
//  Created by Sem Voigtländer on 2/27/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#ifndef simpleserver_h
#define simpleserver_h

#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>

#define SSHELL_MAX_ARG 255
#define SSHELL_ARG_TOKEN_MAX 255
#define SSERV_MAX_CLIENTS 5

typedef struct simple_cli_args {
    int argc;
    char** argv;
    char* tmpTok;
} simple_cli_args;

typedef struct {
    uuid_t id;
    const char* name;
    int fd;
    simple_cli_args args;
    
} simpleserver_client;

typedef struct simpleserver {
    int port;
    struct sockaddr_in addr;
    int srvsock;
    simpleserver_client clients[SSERV_MAX_CLIENTS];
    int numclients;
    const char* servername;
    uuid_t uniqueId;
    bool verbose;
} simpleserver;

typedef enum simpleserver_conn_error {
    E_SIMPLESERV_INV_ARGS,
    E_SIMPLESERV_OK,
    E_SIMPLESERV_FAILURE,
    E_SIMPLESERV_UNKNOWN
} simpleserver_err_t;
simpleserver* simple_server_create(int port, const char* ip, const char* name);
int simple_server_start(simpleserver* server);
int simple_server_stop(simpleserver* server);
int simpleshell_with_interpreter(const char* interpreter, int (*shell_callback)(int argc, char *argv[]), char *cmd);
int simple_connection_handler(simpleserver* server, int (*callback)( int simple_argc, char **simple_argv), const char* interpreter);
#endif /* simpleserver_h */
