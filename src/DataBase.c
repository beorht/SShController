#include "DataBase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

room_list_t db_get_rooms(sqlite3 *db) {
    room_list_t list = { .count = 0 };
    sqlite3_stmt *stmt;

    const char *sql = "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE 'room_%';";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW && list.count < MAX_ROOMS) {
        strncpy(list.rooms[list.count].name,
                (const char *)sqlite3_column_text(stmt, 0), 63);
        list.count++;
    }

    sqlite3_finalize(stmt);
    return list;
}

pc_list_t db_get_by_room(sqlite3 *db, const char *room) {
    pc_list_t list = { .count = 0 };
    sqlite3_stmt *stmt;

    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT name, mac_address, ip FROM %s;", room);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW && list.count < MAX_CLIENTS) {
        pc_entry_t *e = &list.entries[list.count];

        strncpy(e->name, (const char *)sqlite3_column_text(stmt, 0), 31);
        e->name[31] = '\0';

        strncpy(e->mac, (const char *)sqlite3_column_text(stmt, 1), 17);
        e->mac[17] = '\0';

        const char *ip = (const char *)sqlite3_column_text(stmt, 2);
        if (ip) {
            strncpy(e->ip, ip, 15);
            e->ip[15] = '\0';
        } else {
            e->ip[0] = '\0';
        }

        list.count++;
    }

    sqlite3_finalize(stmt);
    return list;
}

pc_list_t db_get_all(sqlite3 *db) {
    pc_list_t list = { .count = 0 };
    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, name, mac_address, ip FROM pc_address;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return list;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW && list.count < MAX_CLIENTS) {
        pc_entry_t *e = &list.entries[list.count];
        e->id = sqlite3_column_int(stmt, 0);

        strncpy(e->name, (const char *)sqlite3_column_text(stmt, 1), 31);
        e->name[31] = '\0';

        strncpy(e->mac, (const char *)sqlite3_column_text(stmt, 2), 17);
        e->mac[17] = '\0';

        const char *ip = (const char *)sqlite3_column_text(stmt, 3);
        if (ip) {
            strncpy(e->ip, ip, 15);
            e->ip[15] = '\0';
        } else {
            e->ip[0] = '\0';
        }

        list.count++;
    }

    sqlite3_finalize(stmt);
    return list;
}

int db_update_ip(sqlite3 *db, const char *room, const char *mac, const char *ip) {
    sqlite3_stmt *stmt;
    char sql[128];
    snprintf(sql, sizeof(sql), "UPDATE %s SET ip = ? WHERE mac_address = ?;", room);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, ip, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, mac, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    return sqlite3_changes(db);
}

void db_free_list(pc_list_t *list) {
    (void)list;
}
