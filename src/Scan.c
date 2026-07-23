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

int filter_by_room(pc_list_t *db_list, scan_device_t *scanned, int scan_count,
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
