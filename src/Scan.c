#include "Scan.h"
#include "Colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

void ping_sweep(const char *subnet) {
    char cmd[128];
    for (int i = 1; i <= 254; i++) {
        snprintf(cmd, sizeof(cmd),
                 "ping -c 1 -W 1 %s.%d > /dev/null 2>&1 &", subnet, i);
        system(cmd);
    }
    sleep(4);
}

int scan_network(scan_device_t *devices, int max) {
    FILE *fp = popen("ip neigh show", "r");
    if (!fp) return 0;

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max) {
        char ip[16], dev[32], mac[18], state[16];

        if (sscanf(line, "%15s dev %31s lladdr %17s %15s",
                   ip, dev, mac, state) >= 3) {
            if (strstr(dev, "lo")) continue;
            if (strcmp(mac, "00:00:00:00:00:00") == 0) continue;

            strcpy(devices[count].ip, ip);
            strcpy(devices[count].mac, mac);
            count++;
        }
    }

    pclose(fp);
    return count;
}

int read_arp_table(device_t *devices, int max) {
    FILE *fp = popen("ip neigh show", "r");
    if (!fp) return 0;

    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max) {
        char ip[16], dev[32], mac[18], state[16];

        if (sscanf(line, "%15s dev %31s lladdr %17s %15s",
                   ip, dev, mac, state) >= 3) {
            if (strstr(dev, "lo")) continue;
            if (strcmp(mac, "00:00:00:00:00:00") == 0) continue;

            strcpy(devices[count].ip, ip);
            strcpy(devices[count].mac, mac);
            strcpy(devices[count].status, state);
            count++;
        }
    }

    pclose(fp);
    return count;
}

static void to_lower_mac(const char *src, char *dst) {
    int i;
    for (i = 0; src[i] && i < 17; i++) {
        dst[i] = tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

int filter_by_room(sqlite3 *db, const char *room, pc_list_t *db_list, scan_device_t *scanned, int scan_count,
                   client_t *clients, const char *key_path, const char *user) {
    int found = 0;

    for (int d = 0; d < db_list->count; d++) {
        char db_mac_lower[18];
        to_lower_mac(db_list->entries[d].mac, db_mac_lower);

        for (int s = 0; s < scan_count; s++) {
            char scan_mac_lower[18];
            to_lower_mac(scanned[s].mac, scan_mac_lower);

            if (strcmp(db_mac_lower, scan_mac_lower) == 0) {
                clients[found].host = strdup(scanned[s].ip);
                clients[found].user = user;
                clients[found].port = 22;
                clients[found].key_path = key_path;
                clients[found].session = NULL;
                found++;

                if (db && db_list->entries[d].ip[0] == '\0') {
                    db_update_ip(db, room, db_list->entries[d].mac, scanned[s].ip);
                }

                printf("  " COLOR_GREEN "Found:" COLOR_RESET " %-14s  IP: " COLOR_CYAN "%-16s" COLOR_RESET "  MAC: " COLOR_YELLOW "%s" COLOR_RESET "\n",
                       db_list->entries[d].name,
                       scanned[s].ip,
                       scanned[s].mac);
                break;
            }
        }
    }

    return found;
}

int save_ips_to_file(client_t *clients, int count, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, COLOR_ERROR "Cannot open file %s for writing\n" COLOR_RESET, filename);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (clients[i].session) {
            fprintf(fp, "%s\n", clients[i].host);
        }
    }

    fclose(fp);
    printf(COLOR_SUCCESS "Saved %d IPs to %s" COLOR_RESET "\n", count, filename);
    return 0;
}
