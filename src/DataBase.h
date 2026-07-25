#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#define MAX_CLIENTS 256
#define MAX_ROOMS   16

typedef struct {
    int     id;
    char    name[32];
    char    mac[18];
    char    ip[16];
} pc_entry_t;

typedef struct {
    pc_entry_t entries[MAX_CLIENTS];
    int        count;
} pc_list_t;

typedef struct {
    char name[64];
} room_t;

typedef struct {
    room_t rooms[MAX_ROOMS];
    int    count;
} room_list_t;

room_list_t db_get_rooms(sqlite3 *db);
pc_list_t   db_get_by_room(sqlite3 *db, const char *room);
pc_list_t   db_get_all(sqlite3 *db);
int         db_update_ip(sqlite3 *db, const char *room, const char *mac, const char *ip);
void        db_free_list(pc_list_t *list);

#endif
