#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Type of Lan
typedef struct {
    char ip[16];
    char mac[18];
    char status[16]; // REACHABLE / STALE / FAILED
} device_t;

// Ping lan
void ping_sweep( const char *subnet ) {
    char cmd[128];
    for ( int i = 1; i <= 254; i++ ) {
        snprintf( cmd, sizeof( cmd ) , "ping -c 1 -W 1 %s.%d > /dev/null 2>&1 &", subnet, i );
        system( cmd );
    }

    sleep(4); // wait for response
}

// Read ARP table
int read_arp_table( device_t *devices, int max ) {
    FILE *fp = popen( "ip neigh show", "r" );

    if ( !fp ) return 0;

    char line[256];
    int count = 0;

    while ( fgets( line, sizeof( line ), fp ) && count < max ) {
        char ip[16], dev[32], mac[18], state[16];

        if ( sscanf( line, "%15s dev %31s lladdr %17s %15s", ip, dev, mac, state ) >= 3 ) {
            // loopback nevalidation
            if ( strstr(dev, "lo") ) continue;
            if ( strcmp(mac, "00:00:00:00:00:00") == 0 ) continue;

            strcpy( devices[count].ip, ip );
            strcpy( devices[count].mac, mac );
            strcpy( devices[count].status, state );

            count++;
        }
    }

    pclose(fp);

    return count;
}

int main( int argc, char *argv[] ) {
    const char *subnet = "192.168.100";

    if ( argc > 1 ) subnet = argv[1];

    printf( "Scanning subnet %s.0/24 ...\n\n", subnet );

    ping_sweep( subnet );

    device_t devices[256];
    int count = read_arp_table( devices, 256 );
    
    printf( "%-16s %-18s %s\n", "IP", "MAC", "Status" );
    printf( "%-16s %-18s %s\n", "---", "---", "---" );

    for ( int i = 0; i < count; i++ ) {
  
        printf( "%-16s %-18s %s\n", 
                devices[i].ip, devices[i].mac, devices[i].status );
    }

    printf( "\nTotal: %d devices\n", count );

    return 0;
}
