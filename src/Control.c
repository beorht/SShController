#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *host;
    const char *user;
    int         port;
    const char *key_path;
    ssh_session session;
} client_t;

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
        fprintf(stderr, "Connection to %s failed: %s\n",
                client->host, ssh_get_error(client->session));
        ssh_free(client->session);
        client->session = NULL;
        return SSH_ERROR;
    }

    int rc = ssh_userauth_publickey_auto(client->session, NULL, NULL);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Auth to %s failed: %s\n",
                client->host, ssh_get_error(client->session));
        ssh_disconnect(client->session);
        ssh_free(client->session);
        client->session = NULL;
        return SSH_ERROR;
    }

    printf("Connected to %s@%s\n", client->user, client->host);
    return SSH_OK;
}

int client_exec(client_t *client, const char *command) {
    if (!client->session) {
        fprintf(stderr, "No session for %s\n", client->host);
        return SSH_ERROR;
    }

    ssh_channel channel = ssh_channel_new(client->session);
    if (!channel) return SSH_ERROR;

    if (ssh_channel_open_session(channel) != SSH_OK) {
        fprintf(stderr, "Channel open failed on %s: %s\n",
                client->host, ssh_get_error(client->session));
        ssh_channel_free(channel);
        return SSH_ERROR;
    }

    if (ssh_channel_request_exec(channel, command) != SSH_OK) {
        fprintf(stderr, "Exec failed on %s: %s\n",
                client->host, ssh_get_error(client->session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }

    char buffer[4096];
    int nbytes;
    printf("--- Output from %s ---\n", client->host);

    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[nbytes] = '\0';
        printf("%s", buffer);
    }

    printf("--- End of %s ---\n\n", client->host);

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_OK;
}

void broadcast_command(client_t *clients, int count, const char *command) {
    for (int i = 0; i < count; i++) {
        if (clients[i].session) {
            printf("Sending to %s (%s)...\n", clients[i].host, clients[i].user);
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

int main(void) {
    client_t clients[] = {
        { "192.168.100.21", "teacher",   22, "/home/thinklinux/.ssh/classroom_agent", NULL },
        { "192.168.100.251", "teacher", 22, "/home/thinklinux/.ssh/classroom_agent", NULL },
    };

    int client_count = sizeof(clients) / sizeof(clients[0]);

    for (int i = 0; i < client_count; i++) {
        client_connect(&clients[i]);
    }

    char command[1024];
    printf("\nConnected to %d clients. Type 'help' for commands, 'exit' to quit:\n\n", client_count);

    while (1) {
        printf(">> ");
        if (!fgets(command, sizeof(command), stdin)) break;

        command[strcspn(command, "\n")] = '\0';

        if (strlen(command) == 0) continue;

        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            break;
        }

        if (strcmp(command, "help") == 0) {
            printf("Commands:\n");
            printf("  <command>  - send command to all clients\n");
            printf("  help       - show this help\n");
            printf("  exit/quit/q - disconnect and exit\n");
            continue;
        }

        broadcast_command(clients, client_count, command);
    }

    for (int i = 0; i < client_count; i++) {
        client_disconnect(&clients[i]);
    }

    return 0;
}
