#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typeof struct {
    int     id;
    char    name[32];
    char    mac[18];
    char    ip[16];
} pc_entry_t;

typedef struct {
    pc_entry_t *entries;
    int         count;
} pc_list_t;

pc_list_t db_get_all( sqlite3 *db ) {
    pc_list_t list = { NULL, 0 };
    sqlite3_stmt *stmt;

    const char *sql = "SELECT id, name, mac_address, ip FROM pc_address;";

    if ( sqlite3_prepare_v2( db, sql, -1, &stmt, NULL ) != SQLITE_OK ) {
        fprintf( stderr, "SQL error: %s\n", sqlite3_errmsg(db) );

        return list;
    }

    int capacity = 64;
    list.entries = malloc( sizeof( pc_entry_t ) * capacity );

    if ( !list.entries ) {
        sqlite3_finalize( stmt );

        return list;
    }

    while ( sqlite3_step( stmt ) == SQLITE_ROW ) {
        if ( list.count >= capacity ) {
            capacity *= 2;
            list.entries = realloc( list.entries, sizeof(pc_entry_t) * capacity );
        }

        pc_entry_t *e = &list.entries[list.count];
        e->id = sqlite3_column_int( stmt, 0 );

        strncpy( e->name, (const char *)sqlite3_column_text(stmt, 1), 31 );
        e->name[31] = '\0';

        strncpy( e->mac, (const char *)sqlite3_column_text(stmt, 2), 17 );
        e->mac[17] = '\0';

        const char *ip = (const char *)sqlite3_column_text(stmt, 3);

        if (ip) {
            strncpy( e->ip, ip, 15 );
            e->ip[15] = '\0';
        } else {
            e->ip[0] = '\0';
        }

        list.count++;
    }

    sqlite3_finalize( stmt );

    return list;
}
