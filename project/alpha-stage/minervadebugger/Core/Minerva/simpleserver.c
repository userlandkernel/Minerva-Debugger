//
//  simpleserver.c
//  NotKDPShell
//
//  Created by Sem Voigtländer on 2/27/19.
//  Copyright © 2019 Sem Voigtländer. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <uuid/uuid.h>

#include "simpleserver.h"

#define SSERV_LAST_CLIENT(server) server->clients[server->numclients-1]



int sshell_nargs = 0;
char* sshell_argv[SSHELL_MAX_ARG];
char* sshell_argv_token;


/* initialize empty nargs/argv struct */
void __attribute__((constructor)) sshell_argv_init()
{
    sshell_nargs = 0;
    sshell_argv_token = calloc(SSHELL_ARG_TOKEN_MAX, sizeof(char));
    bzero(sshell_argv_token, SSHELL_ARG_TOKEN_MAX * sizeof(char));
}

/* add a character to the current token */
void
argv_token_addch(int c)
{
    int n;
    
    n = (int)strlen(sshell_argv_token);
    if (n == SSHELL_ARG_TOKEN_MAX - 1)
    {
        return;
    }
    
    sshell_argv_token[n] = c;
}

/* finish the current token: copy it into _argv and setup next token */
void
argv_token_finish()
{
    if (sshell_nargs == SSHELL_MAX_ARG)
        return;
    sshell_argv[sshell_nargs++] = sshell_argv_token;
    sshell_argv_token = calloc(SSHELL_ARG_TOKEN_MAX, sizeof(char));
    bzero(sshell_argv_token, SSHELL_ARG_TOKEN_MAX * sizeof(char));
}

void sshell_str2argv(char *s)
{
    bool in_token;
    bool in_container;
    bool escaped;
    char container_start;
    char c;
    int  len;
    int  i;
    
    container_start = 0;
    in_token = false;
    in_container = false;
    escaped = false;
    
    len = (int)strlen(s);
    
    for (i = 0; i < len; i++) {
        c = s[i];
        
        switch (c) {
                /* handle whitespace */
            case ' ':
            case '\t':
            case '\n':
                if (!in_token)
                    continue;
                
                if (in_container) {
                    argv_token_addch(c);
                    continue;
                }
                
                if (escaped) {
                    escaped = false;
                    argv_token_addch(c);
                    continue;
                }
                
                /* if reached here, we're at end of token */
                in_token = false;
                argv_token_finish();
                break;
                
                /* handle quotes */
            case '\'':
            case '\"':
                
                if (escaped) {
                    argv_token_addch(c);
                    escaped = false;
                    continue;
                }
                
                if (!in_token) {
                    in_token = true;
                    in_container = true;
                    container_start = c;
                    continue;
                }
                
                if (in_container) {
                    if (c == container_start) {
                        in_container = false;
                        in_token = false;
                        argv_token_finish();
                        continue;
                    } else {
                        argv_token_addch(c);
                        continue;
                    }
                }
                
                /* XXX in this case, we:
                 *    1. have a quote
                 *    2. are in a token
                 *    3. and not in a container
                 * e.g.
                 *    hell"o
                 *
                 * what's done here appears shell-dependent,
                 * but overall, it's an error.... i *think*
                 */
                printf("Parse Error! Bad quotes\n");
                break;
                
            case '\\':
                
                if (in_container && s[i+1] != container_start) {
                    argv_token_addch(c);
                    continue;
                }
                
                if (escaped) {
                    argv_token_addch(c);
                    continue;
                }
                
                escaped = true;
                break;
                
            default:
                if (!in_token) {
                    in_token = true;
                }
                
                argv_token_addch(c);
        }
    }
    
    if (in_container)
        printf("Parse Error! Still in container\n");
    
    if (escaped)
        printf("Parse Error! Unused escape (\\)\n");
}



simpleserver* simple_server_create(int port, const char* ip, const char* name)
{
    simpleserver* server = calloc(1, sizeof(simpleserver));
    bzero(server, sizeof(simpleserver));
    if(ip)
    {
        inet_pton(AF_INET, ip, &server->addr);
    }
    else
    {
        server->addr.sin_family = AF_INET;
        server->addr.sin_addr.s_addr =  htonl(INADDR_ANY);
        server->addr.sin_port = htons(port);
    }
    server->port = port;
    server->servername = name;
    server->numclients = 0;
    uuid_generate((unsigned char*)server->uniqueId);
    server->srvsock = socket(AF_INET, SOCK_STREAM, 0);
    return server;
}

void simple_server_destroy(simpleserver* server)
{
    if(!server)
    {
        printf("BUG: trying to free server that is NULL!!!\n");
    }
    else
    {
        free(server);
    }
}

int simple_connection_welcome(simpleserver* server, const char* welcome_msg)
{
    simpleserver_err_t err = E_SIMPLESERV_OK;
    
    if(!server)
    {
        return E_SIMPLESERV_INV_ARGS;
    }
    
    if(server->srvsock <= 0 || server->numclients <= 0)
    {
        return E_SIMPLESERV_FAILURE;
    }
    
    write(SSERV_LAST_CLIENT(server).fd, (const void*)welcome_msg, strlen(welcome_msg));
    
    return err;
    
}

int simpleshell_with_interpreter(const char* interpreter, int (*shell_callback)(int argc, char *argv[]), char *cmd)
{
    
    sshell_str2argv(cmd);
    if(!sshell_nargs) {
        return 0;
    }
    int ret = shell_callback(sshell_nargs, sshell_argv);
    memset(sshell_argv, 0, sizeof(sshell_argv));
    sshell_nargs = 0;
    sshell_argv_init();
    return ret;
}

int simple_connection_handler(simpleserver* server, int (*callback)( int simple_argc, char **simple_argv), const char* interpreter)
{
    simpleserver_err_t err = E_SIMPLESERV_OK;
    
    if(!server)
    {
        return E_SIMPLESERV_INV_ARGS;
    }
    
    if(!callback)
    {
        return E_SIMPLESERV_INV_ARGS;
    }
    
    char* buffer = malloc(256);
    server->clients[server->numclients].fd = accept(server->srvsock, (struct sockaddr*)NULL, NULL);
    uuid_generate_random((u_char*)&server->clients[server->numclients].id);
    server->clients[server->numclients].name = "";
    server->numclients++;
    char uuid[37];
    uuid_unparse_lower(server->clients[server->numclients-1].id, uuid);
    
    printf("Receiving from: %d (%s)\n", server->clients[server->numclients-1].fd, uuid);
    //pipe stdout and stderr to the socket
    dup2(server->clients[server->numclients-1].fd, fileno(stdout));
    dup2(server->clients[server->numclients-1].fd, fileno(stderr));
    printf("%s", interpreter);
    while(recv(server->clients[server->numclients-1].fd, buffer, 256, 0) > 0)
    {
        
        if(!buffer)
        {
            buffer = malloc(256);
            memset(buffer, '0', 256);
        }
        
        int ret = simpleshell_with_interpreter(interpreter, callback, buffer);
        if(!ret)
        {
            
        }
        if(buffer)
        {
            memset(buffer, 0, 256);
        }
        printf("%s", interpreter);
    }
    
    return err;
}

int simple_connection_listener(simpleserver* server)
{
    simpleserver_err_t err = E_SIMPLESERV_OK;
    
    if(!server)
    {
        return E_SIMPLESERV_INV_ARGS;
    }
    
    int xerr = bind(server->srvsock, (struct sockaddr*)&server->addr, sizeof(struct sockaddr));
    xerr = listen(server->srvsock, 10);
    char uuid[37];
    uuid_unparse_lower(server->uniqueId, uuid);
    printf("server %s(%s) llistens with port %d: %d\n", server->servername, uuid, server->port, xerr);
    
    
    
    
    return err;
}

int simple_client_write(simpleserver_client* client, const char* msg, const char* color)
{
    return 0;
}

int simple_server_write(simpleserver* server, const char* msg, const char* color)
{
    return 0;
}

int simple_server_write_all(simpleserver *server, const char* msg, const char* color)
{
    for(int i = 0; i < server->numclients; i++)
    {
        simpleserver_client* current = &server->clients[i];
        write(current->fd, msg, strlen(msg));
    }
    return 0;
}

int simple_server_list_clients(simpleserver* server)
{
    for(int i = 0; i < server->numclients; i++)
    {
        printf("%d: %s", server->clients[i].fd, server->clients[i].name);
    }
    return 0;
}

int simple_server_start(simpleserver* server)
{
    printf("Creating listener...\n");
    return simple_connection_listener(server);
}

int simple_server_stop(simpleserver* server)
{
    if(!server)
    {
        return E_SIMPLESERV_INV_ARGS;
    }
    
    for(int i = 0; i < server->numclients; i++)
    {
        close(server->clients[i].fd);
    }
    
    close(server->srvsock);
    free(server);
    
    return E_SIMPLESERV_OK;
}
