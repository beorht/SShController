#ifndef SCAN_H
#define SCAN_H

#include "DataBase.h"
#include "Control.h"

typedef struct {
    char ip[16];
    char mac[18];
} scan_device_t;

typedef struct {
    char ip[16];
    char mac[18];
    char status[16];
} device_t;

void ping_sweep(const char *subnet);
int  scan_network(scan_device_t *devices, int max);
int  read_arp_table(device_t *devices, int max);
int  filter_by_room(pc_list_t *db_list, scan_device_t *scanned, int scan_count,
                    client_t *clients, const char *key_path, const char *user);

#endif
