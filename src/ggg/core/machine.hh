#ifndef GGG_CORE_MACHINE_HH
#define GGG_CORE_MACHINE_HH

#include <string>

#include <unistdx/net/ethernet_address>

#include <ggg/core/guile_traits.hh>
#include <ggg/core/host.hh>
#include <ggg/core/ip_address.hh>

namespace ggg {

    class Machine {

    private:
        std::string _name;
        ip_address _ip_address;
        sys::ethernet_address _ethernet_address;

    public:

        Machine() = default;
        ~Machine() = default;
        Machine(const Machine&) = default;
        Machine& operator=(const Machine&) = default;
        Machine(Machine&&) = default;
        Machine& operator=(Machine&&) = default;

        inline
        Machine(const std::string& name,
                const ip_address& ip,
                const sys::ethernet_address& eth):
            _name(name), _ip_address(ip), _ethernet_address(eth) {}

        inline const std::string&
        name() const noexcept {
            return this->_name;
        }

        inline const ip_address&
        address() const noexcept {
            return this->_ip_address;
        }

        inline const sys::ethernet_address&
        ethernet_address() const noexcept {
            return this->_ethernet_address;
        }

        inline void
        swap(Machine& rhs) {
            using std::swap;
            swap(this->_name, rhs._name);
            swap(this->_ip_address, rhs._ip_address);
            swap(this->_ethernet_address, rhs._ethernet_address);
        }

        inline void
        clear() {
            this->_name.clear();
            this->_ip_address.clear();
            this->_ethernet_address.clear();
        }

        inline bool
        operator==(const Machine& rhs) {
            return
                this->_name == rhs._name &&
                this->_ethernet_address == rhs._ethernet_address &&
                this->_ip_address == rhs._ip_address;
        }

        inline bool
        operator!=(const Machine& rhs) {
            return this->operator==(rhs);
        }

        friend std::ostream&
        operator<<(std::ostream& out, const Machine& rhs);

        friend void
        operator>>(const sqlite::statement& in, Machine& rhs);

        friend bool
        operator<(const Machine& lhs, const Machine& rhs);

        friend struct Guile_traits<Machine>;

    };

    inline bool
    operator<(const Machine& lhs, const Machine& rhs) {
        return std::make_tuple(lhs._name, lhs._ip_address, lhs._ethernet_address)
            < std::make_tuple(rhs._name, rhs._ip_address, rhs._ethernet_address);
    }

    std::ostream&
    operator<<(std::ostream& out, const Machine& rhs);

    void
    operator>>(const sqlite::statement& in, Machine& rhs);

    inline void
    swap(Machine& lhs, Machine& rhs) {
        lhs.swap(rhs);
    }

}

#endif // vim:filetype=cpp
