#pragma once

#include <stdio.h>

extern FILE *kmsgfile;

#define KERN_EMERG   "<0>"
#define KERN_ALERT   "<1>"
#define KERN_CRIT    "<2>"
#define KERN_ERR     "<3>"
#define KERN_WARNING "<4>"
#define KERN_NOTICE  "<5>"
#define KERN_INFO    "<6>"
#define KERN_DEBUG   "<7>"

int init_syslog ();
int close_syslog ();

#define atomic_fprintf( file, format, ... ) ( {\
    char msg[ 1024 ]; \
    snprintf( msg, sizeof( msg ), format, ##__VA_ARGS__ ); \
    fprintf( file, "%s", msg ); \
} )

#define printk( format, ... ) \
    atomic_fprintf( kmsgfile, format, ##__VA_ARGS__ )

#define syslog( level, format, ... ) \
    atomic_fprintf( kmsgfile, level PROJECT_NAME ": " format "\n", \
        ##__VA_ARGS__ )
