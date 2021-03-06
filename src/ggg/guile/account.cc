#include <chrono>
#include <ostream>
#include <locale>
#include <ctime>
#include <iomanip>
#include <locale>
#include <ratio>
#include <sstream>
#include <istream>

#include <ggg/core/account.hh>
#include <ggg/core/days.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/store.hh>

namespace {

    template <class T>
    class formatted_date {

    private:
        T _date = nullptr;

    public:
        formatted_date() = default;

        explicit
        formatted_date(T date):
        _date(date)
        {}

        friend std::ostream&
        operator<<(std::ostream& out, const formatted_date& rhs) {
            if (!rhs._date) {
                return out;
            }
            typedef std::time_put<char> tput_type;
            typedef ggg::account::time_point tp;
            typedef ggg::account::duration dur;
            if (*rhs._date > tp(dur::zero())) {
                std::time_t t = ggg::account::clock_type::to_time_t(*rhs._date);
                std::tm* tm = std::gmtime(&t);
                const tput_type& tput = std::use_facet<tput_type>(out.getloc());
                tput.put(out, out, ' ', tm, 'F');
            }
            return out;
        }

        friend std::istream&
        operator>>(std::istream& in, formatted_date& rhs) {
            using ggg::days;
            if (!rhs._date) {
                return in;
            }
            typedef std::time_get<char> tget_type;
            const tget_type& tget = std::use_facet<tget_type>(in.getloc());
            std::ios::iostate state = std::ios::goodbit;
            std::tm tm{};
            std::string tmp;
            in >> tmp;
            std::istringstream iss(tmp);
            std::string fmt("%Y-%m-%d");
            tget.get(iss, tget_type::iter_type(), iss, state, &tm, fmt.data(), fmt.data() + fmt.length());
            std::time_t t = std::mktime(&tm);
            *rhs._date = ggg::account::clock_type::from_time_t(t) + days(1);
            return in;
        }
    };

    template <class T>
    formatted_date<T*>
    make_formatted(T* rhs) {
        return formatted_date<T*>(rhs);
    }

    template <class T>
    formatted_date<const T*>
    make_formatted(const T* rhs) {
        return formatted_date<const T*>(rhs);
    }

    template <class T>
    void
    read_field(T& field, const char* value, const char* err) {
        std::stringstream str(value);
        str >> field;
        if (str.fail()) {
            throw std::invalid_argument(err);
        }
    }

    enum class unit { seconds, minutes, hours, days };

    const char* unit_prefix[4] = { "seconds", "minutes", "hours", "days" };

    class quantity {

    private:
        using rep = std::uint64_t;
        rep _dt = 0;
        unit _unit = unit::seconds;

    public:

        quantity() = default;
        inline explicit quantity(rep dt, unit u = unit::seconds):
        _dt(dt), _unit(u) {}

        template <class T, class Period>
        inline explicit quantity(std::chrono::duration<T,Period> d):
        _dt(std::chrono::duration_cast<std::chrono::seconds>(d).count()),
        _unit(unit::seconds) {}

        inline void normalise() {
            constexpr const unsigned int days = 60*60*24;
            constexpr const unsigned int hours = 60*60;
            constexpr const unsigned int minutes = 60;
            if (_dt % days == 0) { _dt /= days; _unit = unit::days; }
            else if (_dt % hours == 0) { _dt /= hours; _unit = unit::hours; }
            else if (_dt % minutes == 0) { _dt /= minutes; _unit = unit::minutes; }
        }
        inline friend std::ostream&
        operator<<(std::ostream& out, const quantity& rhs) {
            return out << '(' << unit_prefix[int(rhs._unit)] << ' ' << rhs._dt << ')';
        }
    };

}

namespace std {
namespace chrono {

    std::ostream&
    operator<<(std::ostream& out, const ggg::account::time_point& rhs) {
        long d = ggg::to_days(rhs);
        if (d > 0) {
            out << d;
        }
        return out;
    }

    std::ostream&
    operator<<(std::ostream& out, const ggg::account::duration& rhs) {
        long d = ggg::to_days(rhs);
        if (d > 0) {
            out << d;
        }
        return out;
    }

    std::istream&
    operator>>(std::istream& in, ggg::account::time_point& rhs) {
        long val;
        if (!(in >> val)) {
            val = 0L;
            in.clear();
        }
        rhs = ggg::time_point_from_days(val);
        return in;
    }

    std::istream&
    operator>>(std::istream& in, ggg::account::duration& rhs) {
        long val;
        if (!(in >> val)) {
            val = 0L;
            in.clear();
        }
        rhs = ggg::duration_from_days(val);
        return in;
    }

}
}

namespace ggg {

    template <>
    ggg::account
    Guile_traits<ggg::account>::from(SCM obj) {
        using std::chrono::seconds;
        auto s_password = scm_from_latin1_symbol("password");
        auto s_expiration_date = scm_from_latin1_symbol("expiration-date");
        auto s_max_inactive = scm_from_latin1_symbol("max-inactive");
        auto s_name = scm_from_latin1_symbol("name");
        account acc;
        if (slot_is_bound(obj, s_name)) {
            acc._name = to_string(slot(obj, s_name));
        }
        if (slot_is_bound(obj, s_expiration_date)) {
            acc._expire = account::clock_type::from_time_t(
                    scm_to_uint64(slot(obj, s_expiration_date)));
        }
        if (slot_is_bound(obj, s_max_inactive)) {
            acc._maxinactive = seconds(scm_to_uint64(slot(obj, s_max_inactive)));
        }
        if (slot_is_bound(obj, s_password)) {
            auto s = to_string(slot(obj, s_password));
            acc.set_password(account::string(s.begin(), s.end()));
        }
        acc._flags = account_flags(scm_to_uint64(slot(obj, "flags")));
        return acc;
    }

    template <>
    SCM
    Guile_traits<ggg::account>::to(const account& acc) {
        static_assert(std::is_same<uint32_t,sys::uid_type>::value, "bad guile type");
        using std::chrono::duration_cast;
        using std::chrono::seconds;
        return scm_call(
            scm_variable_ref(scm_c_lookup("make")),
            scm_variable_ref(scm_c_lookup("<account>")),
            scm_from_latin1_keyword("name"),
            scm_from_utf8_string(acc.name().data()),
            scm_from_latin1_keyword("expiration-date"),
            scm_from_uint64(account::clock_type::to_time_t(acc.expire())),
            scm_from_latin1_keyword("max-inactive"),
            scm_from_uint64(duration_cast<seconds>(acc.max_inactive()).count()),
            scm_from_latin1_keyword("last-inactive"),
            scm_from_uint64(account::clock_type::to_time_t(acc.last_active())),
            scm_from_latin1_keyword("flags"),
            scm_from_uint64(downcast(acc.flags())),
            SCM_UNDEFINED
        );
    }

    template <>
    void
    Guile_traits<ggg::account>::to_guile(std::ostream& out, const array_type& objects) {
        using std::chrono::duration_cast;
        using std::chrono::seconds;
        const char format[] = "%Y-%m-%dT%H:%M:%S%z";
        if (objects.empty()) { out << "(list)"; return; }
        out << "(list";
        for (const auto& acc : objects) {
            char expire_str[128] {};
            auto t = account::clock_type::to_time_t(acc.expire());
            std::strftime(expire_str, sizeof(expire_str), format, std::localtime(&t));
            std::string flags_str;
            if (acc.flags() & account_flags::suspended) {
                flags_str += " SUSPENDED";
            }
            if (acc.flags() & account_flags::password_has_expired) {
                flags_str += " PASSWORD_HAS_EXPIRED";
            }
            quantity max_inactive(acc.max_inactive());
            max_inactive.normalise();
            out << "\n  ";
            out << "(make <account>\n  ";
            out << "      #:name " << escape_string(acc.name()) << "\n  ";
            out << "      #:expiration-date (time-point " << escape_string(expire_str) << ")\n  ";
            out << "      #:max-inactive " << max_inactive << "\n  ";
            out << "      #:flags (flags" << flags_str << "))";
        }
        out << ")\n";
    }

    template <>
    SCM
    Guile_traits<ggg::account>::insert0(SCM obj) {
        Store store(Store::File::All, Store::Flag::Read_write);
        store.insert(from(obj));
        return SCM_UNSPECIFIED;
    }

    template <>
    SCM
    Guile_traits<ggg::account>::remove(SCM obj) {
        Store store(Store::File::All, Store::Flag::Read_write);
        Transaction tr(store);
        store.erase(from(obj));
        tr.commit();
        return SCM_UNSPECIFIED;
    }

    template <>
    auto
    Guile_traits<ggg::account>::select(std::string name) -> array_type {
        Store store(Store::File::Accounts, Store::Flag::Read_only);
        auto st = store.find_account(name.data());
        return array_type(account_iterator(st), account_iterator());
    }

    template <>
    SCM
    Guile_traits<ggg::account>::find() {
        Store store(Store::File::Accounts, Store::Flag::Read_only);
        auto st = store.accounts();
        account_iterator first(st), last;
        SCM list = SCM_EOL;
        while (first != last) {
            list = scm_append(scm_list_2(list, scm_list_1(to(*first))));
            ++first;
        }
        return list;
    }

    template <>
    auto
    Guile_traits<ggg::account>::generate(const string_array& names) -> array_type {
        std::vector<account> result;
        result.reserve(names.size());
        for (const auto& name : names) {
            result.emplace_back(name.data());
            result.back()._maxinactive = std::chrono::hours(24)*365;
        }
        return result;
    }

}
