#ifndef GGG_CORE_ENTITY_HH
#define GGG_CORE_ENTITY_HH

#include <codecvt>
#include <functional>
#include <istream>
#include <locale>
#include <string>

#include <unistdx/net/bstream>
#include <unistdx/system/nss>

#include <sqlitex/statement.hh>

#include <ggg/core/guile_traits.hh>
#include <ggg/core/protocol_traits.hh>

namespace ggg {

    constexpr const sys::uid_type bad_uid = -1;
    constexpr const sys::gid_type bad_gid = -1;

    class entity {

    public:
        using string_type = std::string;

    public:
        static constexpr const char delimiter = ':';

    private:
        string_type _name;
        string_type _description;
        string_type _homedir;
        string_type _shell;
        string_type _parent;
        sys::uid_type _id = -1;

    public:

        inline explicit entity(const char* name): _name(name) {}
        inline explicit entity(const string_type& name): _name(name) {}

        entity() = default;
        entity(const entity& rhs) = default;
        entity(entity&& rhs) = default;
        ~entity() = default;
        entity& operator=(const entity& rhs) = default;
        entity& operator=(entity&& rhs) = default;

        entity& operator=(const struct ::passwd& rhs);

        inline bool
        operator==(const entity& rhs) const noexcept {
            return name() == rhs.name();
        }

        inline bool
        operator!=(const entity& rhs) const noexcept {
            return !operator==(rhs);
        }

        inline bool
        operator<(const entity& rhs) const noexcept {
            return name() < rhs.name();
        }

        friend std::istream&
        operator>>(std::istream& in, entity& rhs);

        friend std::ostream&
        operator<<(std::ostream& out, const entity& rhs);

        friend sys::bstream&
        operator>>(sys::bstream& in, entity& rhs);

        friend sys::bstream&
        operator<<(sys::bstream& out, const entity& rhs);

        friend void
        operator>>(const sqlite::statement& in, entity& rhs);

        inline sys::uid_type id() const { return this->_id; }
        inline const string_type& name() const { return this->_name; }
        inline const string_type& description() const { return this->_description; }
        inline const string_type& home() const { return this->_homedir; }
        inline const string_type& shell() const { return this->_shell; }
        inline const string_type& parent() const { return this->_parent; }

        inline void id(sys::uid_type id) { this->_id = id; }
        inline void home(const string_type& rhs) { this->_homedir = rhs; }
        inline void shell(const string_type& rhs) { this->_shell = rhs; }
        inline void parent(const string_type& rhs) { this->_parent = rhs; }

        inline bool has_id() const { return this->_id != sys::uid_type(-1); }
        inline bool has_name() const { return !this->_name.empty(); }
        inline bool has_parent() const { return !this->_parent.empty(); }
        bool has_valid_name() const noexcept;

        inline bool
        has_valid_parent() const {
            return this->has_parent() && this->parent() != this->name();
        }

        void clear();

        friend struct Guile_traits<entity>;
        friend struct Protocol_traits<entity>;

    };

    std::istream&
    operator>>(std::istream& in, entity& rhs);

    std::ostream&
    operator<<(std::ostream& out, const entity& rhs);

    sys::bstream&
    operator>>(sys::bstream& in, entity& rhs);

    sys::bstream&
    operator<<(sys::bstream& out, const entity& rhs);

    void
    operator>>(const sqlite::statement& in, entity& rhs);

}

namespace std {

    template <>
    struct hash<ggg::entity>: public hash<string> {

        using result_type = size_t;
        using argument_type = ggg::entity;

        inline result_type
        operator()(const argument_type& rhs) const noexcept {
            return hash<string>::operator()(rhs.name());
        }

    };

}

#endif // vim:filetype=cpp
