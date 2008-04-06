#ifndef _CONFIG_H_
#define _CONFIG_H_

#define PACKAGE                 "vice"
#define VERSION                 "0.16.1.61"
/*#define UNSTABLE*/

#define HAVE_ALLOCA             1
#define RETSIGTYPE              void
#define X_DISPLAY_DEPTH         0
#define HAVE_RESID              1
#define HAS_JOYSTICK            1
#define HAVE_MOUSE		1

#define SIZEOF_UNSIGNED_INT     4
#define SIZEOF_UNSIGNED_LONG    4
#define SIZEOF_UNSIGNED_SHORT   2
#define HAVE_ATEXIT             1
#define HAVE_MEMMOVE            1
#define HAVE_STRERROR           1
#define HAVE_FCNTL_H            1
#define HAVE_LIMITS_H           1
#define HAVE_COMMCTRL_H         1
#define HAVE_SHLOBJ_H           1
#define HAVE_ERRNO_H            1
#define HAVE_IO_H               1
#define HAVE_SYS_TYPES_H        1
#define HAVE_SYS_STAT_H         1
#define HAVE_GUIDLIB            1
#define DWORD_IS_LONG           1

#define __i386__                1

#define inline                  _inline

#define HAVE_UNNAMED_UNIONS     1

#define S_ISDIR(m)              ((m) & _S_IFDIR)

#define MSVC_RC                 1

#endif
