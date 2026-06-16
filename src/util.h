#pragma once

#include <sys/stat.h>
#include <sys/mount.h>

#define S_RW S_IRUSR | S_IWUSR
#define MS_NS MS_NOSUID
#define MS_NSX MS_NOSUID | MS_NOEXEC
#define MS_CLAS MS_NOSUID | MS_NOEXEC | MS_NODEV
#define MS_SCLAS MS_NOSUID | MS_STRICTATIME | MS_NODEV

int reboot_sync ( int howto );
int mkdirr ( const char *path, mode_t mode );
int mountp ( const char *src, const char *target, const char *type,
    int mode, const char *data, int flags );
int fexists ( const char *path );