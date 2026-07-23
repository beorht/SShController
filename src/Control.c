#include "Control.h"
#include "Colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int client_connect(client_t *client) {
    client->session = ssh_new();
    if (!client->session) return SSH_ERROR;

    ssh_options_set(client->session, SSH_OPTIONS_HOST, client->host);
    ssh_options_set(client->session, SSH_OPTIONS_USER, client->user);
    ssh_options_set(client->session, SSH_OPTIONS_PORT, &client->port);
    ssh_options_set(client->session, SSH_OPTIONS_IDENTITY, client->key_path);

    int timeout = 10;
    ssh_options_set(client->session, SSH_OPTIONS_TIMEOUT, &timeout);

    if (ssh_connect(client->session) != SSH_OK) {
        fprintf(stderr, COLOR_ERROR "Connection to %s failed: %s\n" COLOR_RESET,
                client->host, ssh_get_error(client->session));
        ssh_free(client->session);
        client->session = NULL;
        return SSH_ERROR;
    }

    int rc = ssh_userauth_publickey_auto(client->session, NULL, NULL);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, COLOR_ERROR "Auth to %s failed: %s\n" COLOR_RESET,
                client->host, ssh_get_error(client->session));
        ssh_disconnect(client->session);
        ssh_free(client->session);
        client->session = NULL;
        return SSH_ERROR;
    }

    printf(COLOR_SUCCESS "Connected to %s@%s\n" COLOR_RESET, client->user, client->host);
    return SSH_OK;
}

int client_exec(client_t *client, const char *command) {
    if (!client->session) {
        fprintf(stderr, COLOR_ERROR "No session for %s\n" COLOR_RESET, client->host);
        return SSH_ERROR;
    }

    ssh_channel channel = ssh_channel_new(client->session);
    if (!channel) return SSH_ERROR;

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, COLOR_ERROR "Channel open failed on %s: %s\n" COLOR_RESET,
                client->host, ssh_get_error(client->session));
        ssh_channel_free(channel);
        return SSH_ERROR;
    }

    if (ssh_channel_request_exec(channel, command) != SSH_OK) {
        fprintf(stderr, COLOR_ERROR "Exec failed on %s: %s\n" COLOR_RESET,
                client->host, ssh_get_error(client->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }

    char buffer[4096];
    int nbytes;

    printf(COLOR_CLIENT "--- Output from %s ---\n" COLOR_RESET, client->host);

    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[nbytes] = '\0';
        printf(COLOR_CLIENT "%s" COLOR_RESET, buffer);
    }

    printf(COLOR_CLIENT "--- End of %s ---\n\n" COLOR_RESET, client->host);

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_OK;
}

void broadcast_command(client_t *clients, int count, const char *command) {
    for (int i = 0; i < count; i++) {
        if (clients[i].session) {
            printf(COLOR_SERVER "Sending to %s (%s)...\n" COLOR_RESET,
                   clients[i].host, clients[i].user);
            client_exec(&clients[i], command);
        }
    }
}

void client_disconnect(client_t *client) {
    if (client->session) {
        ssh_disconnect(client->session);
        ssh_free(client->session);
        client->session = NULL;
    }
}
