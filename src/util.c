#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/reboot.h>

int reboot_sync ( int howto ) {
    // Sync the fs first, we don't wanna lose data y'now
    sync();

    // Now we can run our reboot command
    reboot( howto );
    return -1;
}

int _mkdir ( const char *path, mode_t mode ) {
    struct stat st;
    errno = 0;

    if ( mkdir( path, mode ) == 0 )
        return 0;

    if (
        errno != EEXIST ||
        stat( path, &st ) != 0
    ) return errno;

    if ( !S_ISDIR( st.st_mode ) )
        return ENOTDIR;

    return 0;
}

int mkdirr ( const char *path, mode_t mode ) {
    errno = 0;
    char *_path = NULL;
    char *p;
    int result = -1;

    _path = strdup( path );
    if ( _path == NULL )
        return false;

    for ( p = _path + 1; *p; p++ ) {
        if ( *p != '/' )
            continue;
        *p = '\0';
        if ( _mkdir( _path, mode ) != 0 )
            goto out;
        *p = '/';
    }

    result = _mkdir( _path, mode );

out:
    free( _path );
    return result;
}

int mountp ( const char *src, const char *target, const char *type,
    int mode, const char *data, int flags ) {
    int resmkdir = mkdirr( target, mode );
    if ( resmkdir != 0 )
        return resmkdir;
    return mount( src, target, type, flags, data );
}

int fexists ( const char *path ) {
    struct stat st;
    return stat( path, &st );
}