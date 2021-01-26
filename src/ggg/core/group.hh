#ifndef GGG_CORE_GROUP_HH
#define GGG_CORE_GROUP_HH

#include <grp.h>
#include <unistd.h>

#include <string>
#include <unordered_set>

#include <unistdx/net/bstream>

#include <sqlitex/statement.hh>

#include <ggg/core/guile_traits.hh>
#include <ggg/core/protocol_traits.hh>

namespace sys {
    typedef ::gid_t gid_type;
}

namespace ggg {

    class group {

    public:
        using char_type = char;
        using string_type = std::string;
        using container_type = std::unordered_set<string_type>;

    private:
        string_type _name;
        sys::gid_type _gid = -1;
        mutable container_type _members;

    public:

        inline explicit
        group(const string_type& name, sys::gid_type gid):
        _name(name), _gid(gid) {}

        inline explicit
        group(
            const string_type& name,
            sys::gid_type gid,
            const container_type& members
        ):
        _name(name),
        _gid(gid),
        _members(members)
        {}

        inline explicit
        group(const string_type& name):
        _name(name)
        {}

        inline explicit
        group(const ::group& rhs):
        _name(rhs.gr_name),
        _gid(rhs.gr_gid)
        {
            for (auto* mem=rhs.gr_mem; *mem; ++mem) {
                this->_members.emplace(*mem);
            }
        }

        group() = default;
        group(const group& rhs) = default;
        group(group&& rhs) = default;
        ~group() = default;
        group& operator=(const group& rhs) = default;
        group& operator=(group&& rhs) = default;

        inline bool has_id() const noexcept { return this->_gid != sys::gid_type(-1); }
        inline bool has_name() const { return !this->_name.empty(); }

        inline bool
        operator==(const group& rhs) const noexcept {
            return name() == rhs.name();
        }

        inline bool
        operator!=(const group& rhs) const noexcept {
            return !operator==(rhs);
        }

        inline bool
        operator<(const group& rhs) const noexcept {
            return name() < rhs.name();
        }

        friend std::ostream&
        operator<<(std::ostream& out, const group& rhs);

        friend sys::bstream&
        operator<<(sys::bstream& out, const group& rhs);

        friend sys::bstream&
        operator>>(sys::bstream& in, group& rhs);

        friend void
        operator>>(const sqlite::statement& in, group& rhs);

        inline sys::gid_type id() const noexcept { return this->_gid; }
        inline const string_type& name() const noexcept { return this->_name; }
        inline container_type& members() const noexcept { return this->_members; }
        inline void members(container_type&& rhs) { this->_members = std::move(rhs); }
        inline void push(const string_type& member) const { this->_members.emplace(member); }
        inline void erase(const string_type& member) const { this->_members.erase(member); }
        inline void clear() const { this->_members.clear(); }

        inline void
        swap(group& rhs) {
            using std::swap;
            swap(this->_name, rhs._name);
            swap(this->_gid, rhs._gid);
            swap(this->_members, rhs._members);
        }

        friend struct Guile_traits<group>;
        friend struct Protocol_traits<group>;

    };

    inline void
    swap(group& lhs, group& rhs) {
        lhs.swap(rhs);
    }

    std::ostream&
    operator<<(std::ostream& out, const group& rhs);

    sys::bstream&
    operator<<(sys::bstream& out, const group& rhs);

    sys::bstream&
    operator>>(sys::bstream& in, group& rhs);

    void
    operator>>(const sqlite::statement& in, group& rhs);

}

namespace std {

    template <>
    struct hash<ggg::group>: public hash<sys::gid_type> {
        inline size_t
        operator()(const ggg::group& rhs) const noexcept {
            return hash<sys::gid_type>::operator()(rhs.id());
        }
    };

}

#endif // vim:filetype=cpp
