#ifndef CONTROL_H
#define CONTROL_H

#include <libssh/libssh.h>

typedef struct {
    const char *host;
    const char *user;
    int         port;
    const char *key_path;
    ssh_session session;
} client_t;

int  client_connect(client_t *client);
int  client_exec(client_t *client, const char *command);
void broadcast_command(client_t *clients, int count, const char *command);
void client_disconnect(client_t *client);

#endif
