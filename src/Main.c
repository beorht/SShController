#include <libssh/libssh.h>
#include <stdio.h>

char *send_command( char *ssh_client_ip, char *ssh_client_username, char *command );

int main( void ) {
    // Connecting and options to client
    ssh_session session = ssh_new();
    ssh_options_set( session, SSH_OPTIONS_HOST, "192.168.100.149" );
    ssh_options_set( session, SSH_OPTIONS_USER, "admin" );
    ssh_options_set( session, SSH_OPTIONS_PORT, &(int){22} );

    // Private key
    ssh_options_set( session, SSH_OPTIONS_IDENTITY, "/home/thinklinux/.ssh/classroom_agent" );

    // Connect
    int rc = ssh_connect( session );

    // Check Connection
    if ( rc != SSH_OK ) {
        printf( "Connection lose" );
        fprintf( stderr, "Connect failed: %s \n", ssh_get_error( session ) );
        
        return 1;
    } else {
        printf( "Connect OK\n" );
    }

    // Auth
    rc = ssh_userauth_publickey_auto( session, NULL, NULL );
    if ( rc != SSH_AUTH_SUCCESS ) {
        fprintf( stderr, "Auth Failed: %s \n", ssh_get_error( session ) );
        ssh_disconnect( session );
        ssh_free( session );

        return 1;
    }
    
    // Отправка команды по ssh
    ssh_channel channel = ssh_channel_new( session );
    rc = ssh_channel_request_exec( channel, "ip -a" );
    
    if ( rc != SSH_OK ) {
        fprintf( stderr, "Request Error: %s \n", ssh_get_error( session ) );
        ssh_channel_close( channel );
        ssh_channel_free( channel );
        ssh_disconnect( session );
        ssh_free( session );
        
        return 1;
    }

    char buffer[256];
    int nbytes;

    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[nbytes] = '\0';
        printf( "Ответ клиента: %s", buffer );
    }

    ssh_channel_send_eof( channel );
    ssh_channel_close( channel );
    ssh_channel_free( channel );
    ssh_disconnect( session );
    ssh_free( session );

    return 0;
}
