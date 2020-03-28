#ifndef GGG_CORE_HOST_ADDRESS_HH
#define GGG_CORE_HOST_ADDRESS_HH

#include <ggg/core/ip_address.hh>

namespace ggg {

    class host_address {

    private:
        ip_address _address;
        std::string _name;

    public:

        host_address() = default;
        ~host_address() = default;
        host_address(const host_address&) = default;
        host_address& operator=(const host_address&) = default;
        host_address(host_address&&) = default;
        host_address& operator=(host_address&&) = default;

        inline explicit
        host_address(const ip_address& address, const std::string& name):
        _address(address),
        _name(name) {}

        inline const ip_address&
        address() const noexcept {
            return this->_address;
        }

        inline const std::string&
        name() const noexcept {
            return this->_name;
        }

        inline void
        clear() {
            this->_name.clear();
            this->_address.clear();
        }

        friend std::ostream&
        operator<<(std::ostream& out, const host_address& rhs);

        friend void
        operator>>(const sqlite::statement& in, host_address& rhs);

    };

    std::ostream&
    operator<<(std::ostream& out, const host_address& rhs);

    void
    operator>>(const sqlite::statement& in, host_address& rhs);

}

#endif // vim:filetype=cpp
