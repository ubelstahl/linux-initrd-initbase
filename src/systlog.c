#include <systlog.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

FILE *kmsgfile = NULL;

int init_syslog () {
    kmsgfile = fopen( "/dev/kmsg", "we" );

    // If we weren't able to open the kmsg, open dev/null instead
    if ( !kmsgfile ) {
        printf( "Warning: fopen /dev/kmsg resulted: %s\n", strerror( errno ) );
        kmsgfile = fopen( "/dev/null", "w" );
        return !kmsgfile ? -1 : 0;
    }

    return setvbuf( kmsgfile, NULL, _IONBF, 0 );
}

int close_syslog () {
    if ( !kmsgfile )
        return 0;
    return fclose( kmsgfile );
}