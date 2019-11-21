#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>

//#define _GNU_SOURCE
#include <dlfcn.h>

#include <iostream>

#include <ggg/config.hh>
#include <ggg/test/override_fs.hh>

#define GGG_DEBUG 0

#ifdef __USE_LARGEFILE64
#define LIBC_NAME(name) name##64
#else
#define LIBC_NAME(name) name
#endif

#define STRINGIFY(s) #s

#define CALL_NEXT(name, ...) \
    reinterpret_cast<system_call_type>(::dlsym(RTLD_NEXT,STRINGIFY(name)))(__VA_ARGS__);

#define LIBC_OPEN LIBC_NAME(open)
#define LIBC_OPENAT LIBC_NAME(openat)
#define LIBC_FSTATAT LIBC_NAME(fstatat)
#define LIBC_FXSTATAT LIBC_NAME(__fxstatat)
#define LIBC_STAT LIBC_NAME(stat)
#define LIBC_XSTAT LIBC_NAME(__xstat)
#define LIBC_LSTAT LIBC_NAME(lstat)
#define LIBC_LXSTAT LIBC_NAME(__lxstat)
#define LIBC_CREAT LIBC_NAME(creat)

typedef int (*system_call_type)(...);

void override_file(const char* func, const char** file) {
    std::string path(*file);
    if (path == GGG_ENTITIES_PATH || path == GGG_WORKDIR "/" GGG_ENTITIES_PATH) {
        *file = GGG_NEW_ENTITIES_PATH;
    }
    if (path == GGG_ACCOUNTS_PATH || path == GGG_WORKDIR "/" GGG_ACCOUNTS_PATH) {
        *file = GGG_NEW_ACCOUNTS_PATH;
    }
    if (path == GGG_GUILE) { *file = GGG_NEW_GUILE; }
    #if GGG_DEBUG
    if (path != *file) { std::clog << func << ' ' << *file << std::endl; }
    #endif
}

extern "C" int LIBC_OPEN(const char* file, int flag, ...) {
    override_file(__func__, &file);
    // from musl libc
    mode_t mode = 0;
    if ((flag & O_CREAT) || (flag & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flag);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return CALL_NEXT(LIBC_OPEN, file, flag, mode);
}

extern "C" int LIBC_OPENAT(int fd, const char* file, int flag, ...) {
    override_file(__func__, &file);
    // from musl libc
    mode_t mode = 0;
    if ((flag & O_CREAT) || (flag & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flag);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return CALL_NEXT(LIBC_OPENAT, fd, file, flag, mode);
}

extern "C" int LIBC_CREAT(const char* file, mode_t mode) {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_CREAT, file, mode);
}

#if defined(HAVE___FXSTATAT)
extern "C" int
LIBC_FXSTATAT(int version, int fd, const char* file, struct LIBC_STAT* buf, int flag) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_FXSTATAT, version, fd, file, buf, flag);
}
#elif defined(HAVE_FSTATAT)
extern "C" int
LIBC_FSTATAT(int fd, const char* file, struct LIBC_STAT* buf, int flag) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_FSTATAT, fd, file, buf, flag);
}
#endif

#if defined(HAVE___XSTAT)
extern "C" int LIBC_XSTAT(int ver, const char* file, struct LIBC_STAT* buf) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_XSTAT, ver, file, buf);
}
#elif defined(HAVE_STAT)
extern "C" int LIBC_STAT(const char* file, struct LIBC_STAT* buf) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_STAT, file, buf);
}
#endif

#if defined(HAVE___LXSTAT)
extern "C" int
LIBC_LXSTAT(int ver, const char* file, struct LIBC_STAT* buf) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_LXSTAT, ver, file, buf);
}
#elif defined(HAVE_LSTAT)
extern "C" int
LIBC_LSTAT(const char* file, struct LIBC_STAT* buf) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(LIBC_LSTAT, file, buf);
}
#endif

extern "C" int remove(const char* file) noexcept {
    override_file(__func__, &file);
    return CALL_NEXT(remove, file);
}
