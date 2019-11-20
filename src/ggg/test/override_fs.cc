#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

//#define _GNU_SOURCE
#include <dlfcn.h>

#include <iostream>

#ifdef __USE_LARGEFILE64
using open64_type = decltype(&::open64);
using openat64_type = decltype(&::openat64);
using creat64_type = decltype(&::creat64);
using fopen64_type = decltype(&::fopen64);
using freopen64_type = decltype(&::freopen64);
using stat64_type = decltype(&::stat64);
using lstat64_type = decltype(&::lstat64);
using fstatat64_type = decltype(&::fstatat64);
#else
using open_type = decltype(&::open);
using openat_type = decltype(&::openat);
using creat_type = decltype(&::creat);
using fopen_type = decltype(&::fopen);
using freopen_type = decltype(&::freopen);
using stat_type = decltype(&::stat);
using lstat_type = decltype(&::lstat);
using fstatat_type = decltype(&::fstatat);
#endif
using remove_type = decltype(&::remove);

#define CALL_NEXT(name, ...) \
    reinterpret_cast<name##_type>(::dlsym(RTLD_NEXT,#name))(__VA_ARGS__);

#ifdef __USE_LARGEFILE64

extern "C" int open64(const char* file, int flag, ...) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(open64, file, flag);
}

extern "C" int openat64(int fd, const char* file, int flag, ...) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(openat64, fd, file, flag);
}

extern "C" int creat64(const char* file, mode_t mode) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(creat64, file, mode);
}

extern "C" FILE* fopen64(const char* file, const char* mode) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(fopen64, file, mode);
}

extern "C" FILE* freopen64(const char* file, const char* mode, FILE* stream) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(freopen64, file, mode, stream);
}

extern "C" int stat64(const char* file, struct stat64* buf) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(stat64, file, buf);
}

extern "C" int lstat64(const char* file, struct stat64* buf) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(lstat64, file, buf);
}

extern "C" int fstatat64(int fd, const char* file, struct stat64* buf, int flag) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(fstatat64, fd, file, buf, flag);
}

#else

extern "C" int open(const char* file, int flag, ...) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(open, file, flag);
}

extern "C" int openat(int fd, const char* file, int flag, ...) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(openat, fd, file, flag);
}

extern "C" int creat(const char* file, mode_t mode) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(creat, file, mode);
}

extern "C" FILE* fopen(const char* file, const char* mode) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(fopen, file, mode);
}

extern "C" FILE* freopen(const char* file, const char* mode, FILE* stream) {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(freopen, file, mode, stream);
}

extern "C" int stat(const char* file, struct stat* buf) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(stat, file, buf);
}

extern "C" int lstat(const char* file, struct stat* buf) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(lstat, file, buf);
}

extern "C" int fstatat(int fd, const char* file, struct stat* buf, int flag) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(fstatat, fd, file, buf, flag);
}

#endif

extern "C" int remove(const char* file) noexcept {
    std::clog << __func__ << ' ' << file << std::endl;
    return CALL_NEXT(remove, file);
}
