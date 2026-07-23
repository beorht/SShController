#include "Control.h"
#include "DataBase.h"
#include "Scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int show_menu(room_list_t *rooms) {
    printf("\n=== Select room ===\n\n");

    for (int i = 0; i < rooms->count; i++) {
        printf("  [%d] %s\n", i + 1, rooms->rooms[i].name);
    }

    printf("\n  [0] Exit\n");

    int choice = -1;
    while (choice < 0 || choice > rooms->count) {
        printf("\nChoice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = -1;
        }
    }

    return choice;
}

int main(int argc, char *argv[]) {
    const char *db_path = "data/database.sqlite";
    const char *key_path = "/home/thinklinux/.ssh/classroom_agent";
    const char *user = "teacher";
    const char *subnet = "192.168.100";
    int do_ping = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--scan") == 0) {
            do_ping = 1;
        } else if (i == 1) {
            db_path = argv[i];
        } else if (i == 2) {
            key_path = argv[i];
        } else if (i == 3) {
            user = argv[i];
        } else if (i == 4) {
            subnet = argv[i];
        }
    }

    sqlite3 *db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    room_list_t rooms = db_get_rooms(db);
    if (rooms.count == 0) {
        fprintf(stderr, "No rooms found in database\n");
        sqlite3_close(db);
        return 1;
    }

    int choice = show_menu(&rooms);
    if (choice == 0) {
        sqlite3_close(db);
        return 0;
    }

    const char *selected_room = rooms.rooms[choice - 1].name;
    printf("\nSelected: %s\n", selected_room);

    pc_list_t room_pcs = db_get_by_room(db, selected_room);
    printf("PCs in database: %d\n", room_pcs.count);

    if (do_ping) {
        printf("\nPinging subnet %s.0/24 ...\n", subnet);
        ping_sweep(subnet);
    }

    printf("\nReading ARP table for %s ...\n", selected_room);
    scan_device_t scanned[256];
    int scan_count = scan_network(scanned, 256);
    printf("Devices in ARP cache: %d\n\n", scan_count);

    printf("Matching MAC addresses:\n");
    client_t clients[MAX_CLIENTS];
    int client_count = filter_by_room(&room_pcs, scanned, scan_count, clients, key_path, user);

    printf("\nMatched PCs: %d\n", client_count);

    if (client_count == 0) {
        fprintf(stderr, "No devices from %s found on network\n", selected_room);
        sqlite3_close(db);
        return 1;
    }

    printf("\nConnecting...\n");
    int connected = 0;
    for (int i = 0; i < client_count; i++) {
        if (client_connect(&clients[i]) == SSH_OK) {
            connected++;
        }
    }

    printf("\nConnected: %d/%d\n\n", connected, client_count);

    if (connected == 0) {
        fprintf(stderr, "No clients connected\n");
        sqlite3_close(db);
        return 1;
    }

    char command[1024];
    printf("Type 'help' for commands, 'exit' to quit:\n\n");

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
            printf("  <command>   - send command to all clients in %s\n", selected_room);
            printf("  list        - show connected clients\n");
            printf("  scan        - show all devices on network\n");
            printf("  help        - show this help\n");
            printf("  exit/quit/q - disconnect and exit\n");
            continue;
        }

        if (strcmp(command, "list") == 0) {
            printf("Connected clients in %s:\n", selected_room);
            for (int i = 0; i < client_count; i++) {
                printf("  [%s] %s - %s\n",
                       clients[i].session ? "ONLINE" : "OFFLINE",
                       clients[i].host,
                       clients[i].user);
            }
            printf("\n");
            continue;
        }

        if (strcmp(command, "scan") == 0) {
            printf("Scanning ARP table...\n\n");
            scan_device_t all_devices[256];
            int all_count = scan_network(all_devices, 256);

            printf("%-4s %-16s %-18s\n", "#", "IP", "MAC");
            printf("%-4s %-16s %-18s\n", "---", "---", "---");

            for (int i = 0; i < all_count; i++) {
                printf("%-4d %-16s %-18s\n",
                       i + 1, all_devices[i].ip, all_devices[i].mac);
            }

            printf("\nTotal: %d devices\n\n", all_count);
            continue;
        }

        broadcast_command(clients, client_count, command);
    }

    for (int i = 0; i < client_count; i++) {
        client_disconnect(&clients[i]);
        free((void *)clients[i].host);
    }

    sqlite3_close(db);
    return 0;
}
