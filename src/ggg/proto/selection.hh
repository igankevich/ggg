#ifndef GGG_PROTO_SELECTION_HH
#define GGG_PROTO_SELECTION_HH

#include <iosfwd>
#include <string>
#include <vector>

#include <unistdx/base/log_message>
#include <unistdx/net/ethernet_address>

#include <ggg/proto/kernel.hh>
#include <ggg/core/ip_address.hh>

namespace ggg {

    class NSS_kernel: public Kernel {

    public:
        enum DB {
            Passwd = 1<<0,
            Group = 1<<1,
            Shadow = 1<<2,
            Hosts = 1<<3,
            Ethers = 1<<4,
        };
        enum Operation {
            Get_all = 1,
            Get_by_name = 2,
            Get_by_id = 3,
            Init_groups = 4,
        };

    private:
        DB _database = Passwd;
        Operation _operation = Get_all;
        std::string _name;
        union {
            sys::uid_type _uid;
            sys::gid_type _gid;
            sys::ethernet_address _ethernet_address{};
            sys::family_type _family;
            ggg::ip_address _ip_address;
        };
        sys::byte_buffer _response{4096};

    public:
        inline NSS_kernel() {}
        inline NSS_kernel(DB db, Operation op): _database(db), _operation(op) {}
        inline void database(DB rhs) { this->_database = rhs; }
        inline void operation(Operation rhs) { this->_operation = rhs; }
        inline void name(std::string rhs) { this->_name = std::move(rhs); }
        inline void uid(sys::uid_type rhs) { this->_uid = rhs; }
        inline void gid(sys::gid_type rhs) { this->_gid = rhs; }
        inline void ethernet_address(const sys::ethernet_address& rhs) {
            this->_ethernet_address = rhs;
        }
        inline void family(sys::family_type rhs) { this->_family = rhs; }
        inline void address(const ip_address& rhs) { this->_ip_address = rhs; }

        template <class T> std::vector<T> response();

        void run() override;
        void read(sys::byte_buffer& buf) override;
        void write(sys::byte_buffer& buf) override;

        template <class ... Args>
        inline void
        log(const char* message, const Args& ... args) const {
            sys::log_message("nss", message, args...);
        }

        void log_request() override;
        void log_response() override;
    };

    const char* to_string(NSS_kernel::DB rhs);
    const char* to_string(NSS_kernel::Operation rhs);
    std::ostream& operator<<(std::ostream& out, NSS_kernel::DB rhs);
    std::ostream& operator<<(std::ostream& out, NSS_kernel::Operation rhs);

}

#endif // vim:filetype=cpp
