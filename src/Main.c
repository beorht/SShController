#include "Control.h"
#include "DataBase.h"
#include "Scan.h"
#include "Colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int show_menu(room_list_t *rooms) {
    printf("\n" COLOR_HIGHLIGHT "=== Select room ===" COLOR_RESET "\n\n");

    for (int i = 0; i < rooms->count; i++) {
        printf("  " COLOR_CYAN "[%d]" COLOR_RESET " %s\n", i + 1, rooms->rooms[i].name);
    }

    printf("\n  " COLOR_RED "[0]" COLOR_RESET " Exit\n");

    int choice = -1;
    while (choice < 0 || choice > rooms->count) {
        printf("\n" COLOR_PROMPT);
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
        fprintf(stderr, COLOR_ERROR "Cannot open database: %s\n" COLOR_RESET, sqlite3_errmsg(db));
        return 1;
    }

    room_list_t rooms = db_get_rooms(db);
    if (rooms.count == 0) {
        fprintf(stderr, COLOR_ERROR "No rooms found in database\n" COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }

    int choice = show_menu(&rooms);
    if (choice == 0) {
        sqlite3_close(db);
        return 0;
    }

    const char *selected_room = rooms.rooms[choice - 1].name;
    printf("\n" COLOR_SERVER "Selected: %s" COLOR_RESET "\n", selected_room);

    pc_list_t room_pcs = db_get_by_room(db, selected_room);
    printf(COLOR_SERVER "PCs in database: %d" COLOR_RESET "\n", room_pcs.count);

    if (do_ping) {
        printf("\n" COLOR_INFO "Pinging subnet %s.0/24 ..." COLOR_RESET "\n", subnet);
        ping_sweep(subnet);
    }

    printf("\n" COLOR_INFO "Reading ARP table for %s ..." COLOR_RESET "\n", selected_room);
    scan_device_t scanned[256];
    int scan_count = scan_network(scanned, 256);
    printf(COLOR_INFO "Devices in ARP cache: %d" COLOR_RESET "\n\n", scan_count);

    printf(COLOR_SERVER "Matching MAC addresses:" COLOR_RESET "\n");
    client_t clients[MAX_CLIENTS];
    int client_count = filter_by_room(db, selected_room, &room_pcs, scanned, scan_count, clients, key_path, user);

    printf("\n" COLOR_SUCCESS "Matched PCs: %d" COLOR_RESET "\n", client_count);

    if (client_count == 0) {
        fprintf(stderr, COLOR_ERROR "No devices from %s found on network\n" COLOR_RESET, selected_room);
        sqlite3_close(db);
        return 1;
    }

    printf("\n" COLOR_INFO "Connecting..." COLOR_RESET "\n");
    int connected = 0;
    char failed_ips[256][16];
    int failed_count = 0;

    for (int i = 0; i < client_count; i++) {
        if (client_connect(&clients[i]) == SSH_OK) {
            connected++;
        } else {
            strncpy(failed_ips[failed_count], clients[i].host, 15);
            failed_ips[failed_count][15] = '\0';
            failed_count++;
        }
    }

    printf("\n" COLOR_SUCCESS "Connected: %d/%d" COLOR_RESET "\n", connected, client_count);

    if (failed_count > 0) {
        printf(COLOR_ERROR "Failed: %d" COLOR_RESET "\n\n", failed_count);

        char failed_filename[256];
        snprintf(failed_filename, sizeof(failed_filename), "%s_failed.txt", selected_room);
        FILE *fp = fopen(failed_filename, "w");
        if (fp) {
            for (int i = 0; i < failed_count; i++) {
                fprintf(fp, "%s\n", failed_ips[i]);
            }
            fclose(fp);
                    printf(COLOR_INFO "Failed IPs saved to: %s" COLOR_RESET "\n", failed_filename);
                    printf(COLOR_HINT "Run: bash scripts/setup_keys.sh -f %s" COLOR_RESET "\n\n", failed_filename);
        }
    }

    if (connected == 0) {
        fprintf(stderr, COLOR_ERROR "No clients connected\n" COLOR_RESET);
        sqlite3_close(db);
        return 1;
    }

    char command[1024];
    printf(COLOR_INFO "Type 'help' for commands, 'exit' to quit:" COLOR_RESET "\n\n");

    while (1) {
        printf(COLOR_PROMPT);
        if (!fgets(command, sizeof(command), stdin)) break;

        command[strcspn(command, "\n")] = '\0';

        if (strlen(command) == 0) continue;

        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            break;
        }

        if (strcmp(command, "help") == 0) {
            printf(COLOR_HIGHLIGHT "Commands:" COLOR_RESET "\n");
            printf("  " COLOR_CYAN "<command>" COLOR_RESET "   - send command to all clients in " COLOR_YELLOW "%s" COLOR_RESET "\n", selected_room);
            printf("  " COLOR_CYAN "list" COLOR_RESET "        - show connected clients\n");
            printf("  " COLOR_CYAN "scan" COLOR_RESET "        - show all devices on network\n");
            printf("  " COLOR_CYAN "reconnect" COLOR_RESET "  - rescan and reconnect to clients\n");
            printf("  " COLOR_CYAN "saveips" COLOR_RESET "     - save connected IPs to file\n");
            printf("  " COLOR_CYAN "help" COLOR_RESET "        - show this help\n");
            printf("  " COLOR_RED "exit/quit/q" COLOR_RESET " - disconnect and exit\n");
            continue;
        }

        if (strcmp(command, "list") == 0) {
            printf(COLOR_HIGHLIGHT "Connected clients in %s:" COLOR_RESET "\n", selected_room);
            for (int i = 0; i < client_count; i++) {
                if (clients[i].session) {
                    printf("  " COLOR_GREEN "[%s]" COLOR_RESET " %s - %s\n",
                           "ONLINE", clients[i].host, clients[i].user);
                } else {
                    printf("  " COLOR_RED "[%s]" COLOR_RESET " %s - %s\n",
                           "OFFLINE", clients[i].host, clients[i].user);
                }
            }
            printf("\n");
            continue;
        }

        if (strcmp(command, "scan") == 0) {
            printf(COLOR_INFO "Scanning ARP table..." COLOR_RESET "\n\n");
            scan_device_t all_devices[256];
            int all_count = scan_network(all_devices, 256);

            printf(COLOR_HIGHLIGHT "%-4s %-16s %-18s" COLOR_RESET "\n", "#", "IP", "MAC");
            printf("%-4s %-16s %-18s\n", "---", "---", "---");

            for (int i = 0; i < all_count; i++) {
                printf("%-4d " COLOR_CYAN "%-16s" COLOR_RESET " " COLOR_YELLOW "%-18s" COLOR_RESET "\n",
                       i + 1, all_devices[i].ip, all_devices[i].mac);
            }

            printf("\n" COLOR_SUCCESS "Total: %d devices" COLOR_RESET "\n\n", all_count);
            continue;
        }

        if (strcmp(command, "reconnect") == 0) {
            printf(COLOR_INFO "Rescanning network..." COLOR_RESET "\n\n");

            if (do_ping) {
                printf(COLOR_INFO "Pinging subnet %s.0/24 ..." COLOR_RESET "\n", subnet);
                ping_sweep(subnet);
            }

            printf(COLOR_INFO "Reading ARP table..." COLOR_RESET "\n");
            scan_device_t new_scanned[256];
            int new_scan_count = scan_network(new_scanned, 256);
            printf(COLOR_INFO "Devices in ARP cache: %d" COLOR_RESET "\n\n", new_scan_count);

            printf(COLOR_SERVER "Matching MAC addresses:" COLOR_RESET "\n");

            for (int i = 0; i < client_count; i++) {
                client_disconnect(&clients[i]);
                free((void *)clients[i].host);
            }

            client_count = filter_by_room(db, selected_room, &room_pcs, new_scanned, new_scan_count, clients, key_path, user);

            printf("\n" COLOR_SUCCESS "Matched PCs: %d" COLOR_RESET "\n", client_count);

            if (client_count == 0) {
                fprintf(stderr, COLOR_ERROR "No devices found on network\n" COLOR_RESET);
                continue;
            }

            printf(COLOR_INFO "Connecting..." COLOR_RESET "\n");
            int reconnected = 0;
            char re_failed_ips[256][16];
            int re_failed_count = 0;

            for (int i = 0; i < client_count; i++) {
                if (client_connect(&clients[i]) == SSH_OK) {
                    reconnected++;
                } else {
                    strncpy(re_failed_ips[re_failed_count], clients[i].host, 15);
                    re_failed_ips[re_failed_count][15] = '\0';
                    re_failed_count++;
                }
            }

            printf("\n" COLOR_SUCCESS "Reconnected: %d/%d" COLOR_RESET "\n", reconnected, client_count);

            if (re_failed_count > 0) {
                printf(COLOR_ERROR "Failed: %d" COLOR_RESET "\n\n", re_failed_count);

                char failed_filename[256];
                snprintf(failed_filename, sizeof(failed_filename), "%s_failed.txt", selected_room);
                FILE *fp = fopen(failed_filename, "w");
                if (fp) {
                    for (int i = 0; i < re_failed_count; i++) {
                        fprintf(fp, "%s\n", re_failed_ips[i]);
                    }
                    fclose(fp);
                    printf(COLOR_INFO "Failed IPs saved to: %s" COLOR_RESET "\n", failed_filename);
                    printf(COLOR_HINT "Run: bash scripts/setup_keys.sh -f %s" COLOR_RESET "\n\n", failed_filename);
                }
            }
            continue;
        }

        if (strcmp(command, "saveips") == 0) {
            char filename[256];
            snprintf(filename, sizeof(filename), "%s_ips.txt", selected_room);
            save_ips_to_file(clients, client_count, filename);
            printf("\n");
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
