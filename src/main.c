#include <systlog.h>
#include <util.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#include <sys/utsname.h>
#include <sys/reboot.h>
#include <sys/wait.h>

#define spawnwait( path, ... ) ( {\
    int status = -1;  \
    pid_t pid = fork(); \
    if ( pid == 0 ) \
        execl( path, ##__VA_ARGS__, NULL ); \
    else if ( pid == -1 ) { \
        syslog( KERN_EMERG, "Could not run %s. (fork resulted -1)", path ); \
        reboot_sync( RB_HALT_SYSTEM ); \
    } else waitpid( pid, &status, 0 ); \
    status; \
} )

void psighandler ( int sig ) {
    switch ( sig ) {
        case SIGTERM:
            reboot_sync( RB_AUTOBOOT );
            break;
        case SIGUSR2:
            reboot_sync( RB_POWER_OFF );
            break;
    }
}

int main () {
    // First of all, check if we are being ran as PID 1
    if ( getpid() != 1 ) {
        printf( "%s version %s\n", PROJECT_NAME, PROJECT_VERSION );
        printf( "Open-Source, Public Domain. 2026 %s\n\n", PROJECT_AUTHOR );
        printf( "Run this as PID 1... Pretty please with a cherry on top?\n" );
    }

    // Mount /dev so we can do syslog/printk whatever you call it
    if ( mountp( "devtmpfs", "/dev", "devtmpfs", S_RW, "mode=0755", MS_NS ) )
        return -1; // Make the kernel panic idc

    // Finally init the syslog
    init_syslog();

    // Greet thy console!!!
    printk( KERN_WARNING "%s version %s", PROJECT_NAME, PROJECT_VERSION );

    // Mount the rest
    mountp( "sysfs", "/sys", "sysfs", S_RW, "", MS_CLAS );
    mountp( "proc", "/proc", "proc", S_RW, "", MS_CLAS );
    mountp( "run", "/run", "tmpfs", S_RW, "", MS_SCLAS );
    mountp( "devpts", "/dev/pts", "devpts", S_RW, "gid=5,mode=0620", MS_NSX );

    // If the system is in EFI mode, mount efivarfs
    if ( fexists( "/sys/firmware/efi" ) == true )
        mountp( "efivarfs", "/sys/firmware/efi/efivars", "efivarfs", S_RW,
            "", MS_CLAS );

    // Set PATH so executables can find other executables etc.
    setenv( "PATH", "/usr/bin:/usr/sbin:/usr/local/bin:/bin:/sbin", 1 );

    // Get kernel information
    struct utsname utsbuf;
    if ( uname( &utsbuf ) != 0 ) {
        // You can change this part if you want kernel information to be
        // optional or whatever, depending on your usage.
        syslog( KERN_EMERG, "Could not load kernel information." );
        reboot_sync( RB_HALT_SYSTEM );
    }

    // You can load modules, run services or handover the init procedure
    // to another executable, You have infinite choices and combinations.
    // Personally I'm just gonna hand it over to busybox.

    // Now we are going to open up TTY and for that we're gonna send TIOCGDEV
    // to /dev/console, get it's maj/min and create a node at /dev/ttySYSG
    int confd = open( "/dev/console", O_RDWR );
    if ( confd < 0 )
        goto busybox;

    // Get the dev num
    unsigned int devnum = 0;
    if ( ioctl( confd, TIOCGDEV, &devnum ) < 0 ) {
        syslog( KERN_EMERG, "Could not get controlling TTY behind console." );
        close( confd );
        reboot_sync( RB_HALT_SYSTEM );
    }

    // We don't need the console now
    close( confd );

    // Prepare path
    const char *systtypath = "/dev/ttySYSG";
    unlink( systtypath );

    // Prepare node
    if ( mknod(
        systtypath,
        S_IFCHR | 0660,
        makedev( major( devnum ), minor( devnum ) )
    ) < 0 ) {
        syslog( KERN_EMERG, "Could not create system TTY node." );
        reboot_sync( RB_HALT_SYSTEM );
    }

    // Create a new session so the TTY instances can be opened as CTTY
    // by default.
    setsid();

    // Open that fake node and forward standard IO
    int ttyfd = open( systtypath , O_RDWR );
    if ( ttyfd < 0 ) {
        syslog( KERN_EMERG, "Could not open system TTY node." );
        unlink( systtypath );
        reboot_sync( RB_HALT_SYSTEM );
    }

    dup2( ttyfd, 0 );
    dup2( ttyfd, 1 );
    dup2( ttyfd, 2 );

    if ( ttyfd > 2 )
        close( ttyfd );

    // Cleanup :3
    unlink( systtypath );

busybox:
    // Handover to busybox
    mkdirr( "/bin", S_IRWXU );

    if ( fexists( "/bin/busybox" ) != 0 ) {
        syslog( KERN_EMERG, "Could not switch to busybox "
            "(/bin/busybox does not exist.)" );
        reboot_sync( RB_HALT_SYSTEM );
    }

    // Before handing the init procedure to something or switching root,
    // make sure to close /dev/kmsg we opened earlier
    close_syslog();

    // Register signals so busybox can properly restart and power off
    // the system.
    signal( SIGTERM, psighandler );
    signal( SIGUSR2, psighandler );

    spawnwait( "/bin/busybox", "busybox", "--install", "-s", "/bin" );

#ifndef BUSYBOX_EXIT_MEANS_REBOOT
    // Busybox shall not exit...
    while ( 1 ) {
#endif
        spawnwait( "/bin/busybox", "sh" );
#ifndef BUSYBOX_EXIT_MEANS_REBOOT
        printf(
            "Hey, You can't exit. If you want to reboot or power-off\n"
            "the system, you should use 'poweroff' or 'reboot' commands.\n"
        );
    }
#endif
    reboot_sync( RB_AUTOBOOT );
    return 0;
}