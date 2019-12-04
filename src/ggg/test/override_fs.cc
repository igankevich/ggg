#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

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
#define LIBC_MKDIR mkdir
#define LIBC_MKDIRAT mkdirat

typedef int (*system_call_type)(...);

std::string override_file(const char* func, const char* file) {
    using traits_type = std::string::traits_type;
    std::string path(file);
    if (path == GGG_ENTITIES_PATH) {
        path.clear();
        if (const char* suffix = std::getenv("GGG_TEST_SUFFIX")) {
            path += "tmp/";
            mkdir(path.data(), 0755);
            path += suffix;
            path += '/';
            mkdir(path.data(), 0755);
        }
        path += "entities.tmp";
    }
    if (path == GGG_ACCOUNTS_PATH) {
        path.clear();
        if (const char* suffix = std::getenv("GGG_TEST_SUFFIX")) {
            path += "tmp/";
            mkdir(path.data(), 0755);
            path += suffix;
            path += '/';
            mkdir(path.data(), 0755);
        }
        path += "accounts.tmp";
    }
    { std::clog << func << ' ' << path << std::endl; }
    if (path == GGG_GUILE) { path = GGG_NEW_GUILE; }
    if (path == GGG_ROOT) {
        path = GGG_WORKDIR;
        if (const char* suffix = std::getenv("GGG_TEST_SUFFIX")) {
            path += "tmp/";
            mkdir(path.data(), 0755);
            path += suffix;
            path += '/';
            mkdir(path.data(), 0755);
        }
    }
    auto n = traits_type::length(GGG_ROOT);
    if (path.compare(0, n, GGG_ROOT, n) == 0) {
        auto tmp = path.substr(n+1);
        path.clear();
        if (const char* suffix = std::getenv("GGG_TEST_SUFFIX")) {
            path += "tmp/";
            mkdir(path.data(), 0755);
            path += suffix;
            path += '/';
            mkdir(path.data(), 0755);
        }
        path += tmp;
    }
    if (path == GGG_NEW_GUILE_LOAD_PATH "/ggg/types.scm" ||
        path == GGG_GUILE_LOAD_PATH "/ggg/types.scm") {
        path = GGG_NEW_GUILE_LOAD_PATH "/ggg/types_test.scm";
    }
    #if GGG_DEBUG
    if (path != file) { std::clog << func << ' ' << path << std::endl; }
    #endif
    return path;
}

extern "C" int LIBC_OPEN(const char* file, int flag, ...) {
    auto new_file = override_file(__func__, file);
    // from musl libc
    mode_t mode = 0;
    if ((flag & O_CREAT) || (flag & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flag);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return CALL_NEXT(LIBC_OPEN, new_file.data(), flag, mode);
}

extern "C" int LIBC_OPENAT(int fd, const char* file, int flag, ...) {
    auto new_file = override_file(__func__, file);
    // from musl libc
    mode_t mode = 0;
    if ((flag & O_CREAT) || (flag & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flag);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return CALL_NEXT(LIBC_OPENAT, fd, new_file.data(), flag, mode);
}

extern "C" int LIBC_CREAT(const char* file, mode_t mode) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_CREAT, new_file.data(), mode);
}

#if defined(HAVE___FXSTATAT)
extern "C" int
LIBC_FXSTATAT(int version, int fd, const char* file, struct LIBC_STAT* buf, int flag) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_FXSTATAT, version, fd, new_file.data(), buf, flag);
}
#elif defined(HAVE_FSTATAT)
extern "C" int
LIBC_FSTATAT(int fd, const char* file, struct LIBC_STAT* buf, int flag) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_FSTATAT, fd, new_file.data(), buf, flag);
}
#endif

#if defined(HAVE___XSTAT)
extern "C" int LIBC_XSTAT(int ver, const char* file, struct LIBC_STAT* buf) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_XSTAT, ver, new_file.data(), buf);
}
#endif

#if defined(HAVE_STAT)
extern "C" int LIBC_STAT(const char* file, struct LIBC_STAT* buf) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_STAT, new_file.data(), buf);
}
#endif

#if defined(HAVE___LXSTAT)
extern "C" int
LIBC_LXSTAT(int ver, const char* file, struct LIBC_STAT* buf) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_LXSTAT, ver, new_file.data(), buf);
}
#elif defined(HAVE_LSTAT)
extern "C" int
LIBC_LSTAT(const char* file, struct LIBC_STAT* buf) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_LSTAT, new_file.data(), buf);
}
#endif

extern "C" int remove(const char* file) noexcept {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(remove, new_file.data());
}

extern "C" int LIBC_MKDIR(const char* file, mode_t mode) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_MKDIR, new_file.data(), mode);
}

extern "C" int LIBC_MKDIRAT(int fd, const char* file, mode_t mode) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(LIBC_MKDIRAT, fd, new_file.data(), mode);
}

extern "C" int chown(const char* file, uid_t owner, gid_t group) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(chown, new_file.data(), owner, group);
}

extern "C" int lchown(const char* file, uid_t owner, gid_t group) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(lchown, new_file.data(), owner, group);
}

extern "C" int fchownat(int fd, const char* file, uid_t owner, gid_t group, int flag) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(fchownat, fd, new_file.data(), owner, group, flag);
}

extern "C" int chmod(const char* file, mode_t mode) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(chmod, new_file.data(), mode);
}

extern "C" int lchmod(const char* file, mode_t mode) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(lchmod, new_file.data(), mode);
}

extern "C" int fchmodat(int fd, const char* file, mode_t mode, int flag) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(fchmodat, fd, new_file.data(), mode, flag);
}

extern "C" int unlink(const char* file) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(unlink, new_file.data());
}

extern "C" int unlinkat(int fd, const char* file, int flag) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(unlinkat, fd, new_file.data(), flag);
}

extern "C" int rmdir(const char* file) {
    auto new_file = override_file(__func__, file);
    return CALL_NEXT(rmdir, new_file.data());
}
