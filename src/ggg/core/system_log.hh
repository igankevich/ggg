#ifndef GGG_CORE_SYSTEM_LOG_HH
#define GGG_CORE_SYSTEM_LOG_HH

#include <syslog.h>

#include <unistdx/base/flag>

namespace sys {

    class log {

    public:

        enum class options: int {
            #if defined(LOG_PID)
            pid=LOG_PID,
            #endif
            #if defined(LOG_CONS)
            console=LOG_CONS,
            #endif
            #if defined(LOG_ODELAY)
            delay_open=LOG_ODELAY,
            #endif
            #if defined(LOG_NDELAY)
            do_not_delay_open=LOG_NDELAY,
            #endif
            #if defined(LOG_PERROR)
            stderr=LOG_PERROR,
            #endif
        };

        enum class facilities: int {
            #if defined(LOG_KERN)
            kern=LOG_KERN,
            #endif
            #if defined(LOG_USER)
            user=LOG_USER,
            #endif
            #if defined(LOG_MAIL)
            mail=LOG_MAIL,
            #endif
            #if defined(LOG_DAEMON)
            daemon=LOG_DAEMON,
            #endif
            #if defined(LOG_AUTH)
            auth=LOG_AUTH,
            #endif
            #if defined(LOG_SYSLOG)
            syslog=LOG_SYSLOG,
            #endif
            #if defined(LOG_LPR)
            lpr=LOG_LPR,
            #endif
            #if defined(LOG_NEWS)
            news=LOG_NEWS,
            #endif
            #if defined(LOG_UUCP)
            uucp=LOG_UUCP,
            #endif
            #if defined(LOG_CRON)
            cron=LOG_CRON,
            #endif
            #if defined(LOG_AUTHPRIV)
            authpriv=LOG_AUTHPRIV,
            #endif
            #if defined(LOG_FTP)
            ftp=LOG_FTP,
            #endif
            #if defined(LOG_LOCAL0)
            local0=LOG_LOCAL0,
            #endif
            #if defined(LOG_LOCAL1)
            local1=LOG_LOCAL1,
            #endif
            #if defined(LOG_LOCAL2)
            local2=LOG_LOCAL2,
            #endif
            #if defined(LOG_LOCAL3)
            local3=LOG_LOCAL3,
            #endif
            #if defined(LOG_LOCAL4)
            local4=LOG_LOCAL4,
            #endif
            #if defined(LOG_LOCAL5)
            local5=LOG_LOCAL5,
            #endif
            #if defined(LOG_LOCAL6)
            local6=LOG_LOCAL6,
            #endif
            #if defined(LOG_LOCAL7)
            local7=LOG_LOCAL7,
            #endif
        };

        enum class levels: int {
            #if defined(LOG_EMERG)
            emergency=LOG_EMERG,
            #endif
            #if defined(LOG_ALERT)
            alert=LOG_ALERT,
            #endif
            #if defined(LOG_CRIT)
            critical=LOG_CRIT,
            #endif
            #if defined(LOG_ERR)
            error=LOG_ERR,
            #endif
            #if defined(LOG_WARNING)
            warning=LOG_WARNING,
            #endif
            #if defined(LOG_NOTICE)
            notice=LOG_NOTICE,
            #endif
            #if defined(LOG_INFO)
            information=LOG_INFO,
            #endif
            #if defined(LOG_DEBUG)
            debug=LOG_DEBUG,
            #endif
        };

        enum class priority: int {};

        inline explicit
        log(const char* name=nullptr,
            facilities facility=facilities::user,
            options option=options{}) noexcept {
            ::openlog(name, int(option), int(facility));
        }

        inline ~log() noexcept { ::closelog(); }

        static inline levels max_level(levels rhs) noexcept {
            return levels(::setlogmask(LOG_UPTO(int(rhs))));
        }

    };

    UNISTDX_FLAGS(log::options);

    constexpr inline log::priority operator|(log::facilities a, log::levels b) noexcept {
        using type1 = typename std::underlying_type<log::facilities>::type;
        using type2 = typename std::underlying_type<log::levels>::type;
        return log::priority(type1(a) | type2(b));
    }

    constexpr inline log::priority operator|(log::levels a, log::facilities b) noexcept {
        using type1 = typename std::underlying_type<log::facilities>::type;
        using type2 = typename std::underlying_type<log::levels>::type;
        return log::priority(type2(a) | type1(b));
    }

    template <class ... Args> inline void
    message(log::priority prio, const char* format, Args ... args) noexcept {
        ::syslog(int(prio), format, args...);
    }

    template <class ... Args> inline void
    message(const char* format, Args ... args) noexcept {
        message(log::facilities::user | log::levels::information, format, args...);
    }

    template <class ... Args> inline void
    message(log::facilities facility, log::levels level,
            const char* format, Args ... args) noexcept {
        message(facility | level, format, args...);
    }

}

#endif // vim:filetype=cpp
