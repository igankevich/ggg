#include <ggg/core/database.hh>
#include <ggg/proto/selection.hh>
#include <ggg/proto/traits.hh>

namespace ggg {

    template <>
    void Protocol_traits<std::string>::write(sys::byte_buffer& buf, const std::string& rhs) {
        buf.write(rhs);
    }

    template <>
    void Protocol_traits<std::string>::read(sys::byte_buffer& buf, std::string& rhs) {
        buf.read(rhs);
    }

    template <>
    void Protocol_traits<entity>::write(sys::byte_buffer& buf, const entity& rhs) {
        buf.write(rhs._name);
        buf.write(rhs._description);
        buf.write(rhs._homedir);
        buf.write(rhs._shell);
        buf.write(rhs._id);
    }

    template <>
    void Protocol_traits<entity>::read(sys::byte_buffer& buf, entity& rhs) {
        buf.read(rhs._name);
        buf.read(rhs._description);
        buf.read(rhs._homedir);
        buf.read(rhs._shell);
        buf.read(rhs._id);
    }

    template <>
    void Protocol_traits<group>::write(sys::byte_buffer& buf, const group& rhs) {
        buf.write(rhs._name);
        buf.write(rhs._gid);
        buf.write(static_cast<sys::u32>(rhs._members.size()));
        for (const auto& m : rhs._members) { buf.write(m); }
    }

    template <>
    void Protocol_traits<group>::read(sys::byte_buffer& buf, group& rhs) {
        buf.read(rhs._name);
        buf.read(rhs._gid);
        sys::u32 nmembers = 0;
        buf.read(nmembers);
        for (sys::u32 i=0; i<nmembers; ++i) {
            std::string name;
            buf.read(name);
            rhs._members.emplace(std::move(name));
        }
    }

    template <>
    void Protocol_traits<account>::write(sys::byte_buffer& buf, const account& rhs) {
        buf.write(rhs._name);
    }

    template <>
    void Protocol_traits<account>::read(sys::byte_buffer& buf, account& rhs) {
        buf.read(rhs._name);
    }

    template <>
    void Protocol_traits<host>::write(sys::byte_buffer& buf, const host& rhs) {
        buf.write(rhs._address.data(), rhs._address.size());
        buf.write(rhs._name);
    }

    template <>
    void Protocol_traits<host>::read(sys::byte_buffer& buf, host& rhs) {
        buf.read(rhs._address.data(), rhs._address.size());
        buf.read(rhs._name);
    }

    template <>
    void Protocol_traits<ip_address>::write(sys::byte_buffer& buf, const ip_address& rhs) {
        buf.write(rhs._family);
        if (rhs._family == sys::family_type::inet) {
            buf.write(rhs._address4.data(), rhs._address4.size());
        } else {
            buf.write(rhs._address6.data(), rhs._address6.size());
        }
    }

    template <>
    void Protocol_traits<ip_address>::read(sys::byte_buffer& buf, ip_address& rhs) {
        buf.read(rhs._family);
        if (rhs._family == sys::family_type::inet) {
            buf.read(rhs._address4.data(), rhs._address4.size());
        } else {
            buf.read(rhs._address6.data(), rhs._address6.size());
        }
    }

    template <>
    void Protocol_traits<host_address>::write(sys::byte_buffer& buf, const host_address& rhs) {
        Protocol_traits<ip_address>::write(buf, rhs._address);
        buf.write(rhs._name);
    }

    template <>
    void Protocol_traits<host_address>::read(sys::byte_buffer& buf, host_address& rhs) {
        Protocol_traits<ip_address>::read(buf, rhs._address);
        buf.read(rhs._name);
    }

    template <>
    void Protocol_traits<public_key>::write(sys::byte_buffer& buf, const public_key& rhs) {
        buf.write(rhs._name);
        buf.write(rhs._options);
        buf.write(rhs._type);
        buf.write(rhs._key);
        buf.write(rhs._comment);
    }

    template <>
    void Protocol_traits<public_key>::read(sys::byte_buffer& buf, public_key& rhs) {
        buf.read(rhs._name);
        buf.read(rhs._options);
        buf.read(rhs._type);
        buf.read(rhs._key);
        buf.read(rhs._comment);
    }

    const char* to_string(NSS_kernel::DB rhs) {
        switch (rhs) {
            case NSS_kernel::DB::Passwd: return "passwd";
            case NSS_kernel::DB::Group: return "group";
            case NSS_kernel::DB::Shadow: return "shadow";
            case NSS_kernel::DB::Hosts: return "hosts";
            case NSS_kernel::DB::Ethers: return "ethers";
            case NSS_kernel::DB::Public_keys: return "public-keys";
            default: return "unknown";
        }
    }

    const char* to_string(NSS_kernel::Operation rhs) {
        switch (rhs) {
            case NSS_kernel::Operation::Get_all: return "get-all";
            case NSS_kernel::Operation::Get_by_name: return "get-by-name";
            case NSS_kernel::Operation::Get_by_id: return "get-by-id";
            case NSS_kernel::Operation::Init_groups: return "init-groups";
            default: return "unknown";
        }
    }

    std::ostream& operator<<(std::ostream& out, NSS_kernel::DB rhs) {
        return out << to_string(rhs);
    }

    std::ostream& operator<<(std::ostream& out, NSS_kernel::Operation rhs) {
        return out << to_string(rhs);
    }

}

namespace {

    template <class T>
    void to_bytes(sys::byte_buffer& buf, sqlite::statement st) {
        for (const auto& entry : st.rows<T>()) {
            sys::log_message("nss", "entity _", entry);
            ggg::Protocol_traits<T>::write(buf, entry);
        }
    }

    template <class T>
    void to_bytes(sys::byte_buffer& buf, ggg::Database::group_container_t groups) {
        for (const auto& pair : groups) {
            ggg::Protocol_traits<T>::write(buf, pair.second);
        }
    }

    template <class T>
    std::vector<T> from_bytes(sys::byte_buffer& buf) {
        std::vector<T> tmp;
        T entry;
        while (buf.remaining() != 0) {
            ggg::Protocol_traits<T>::read(buf, entry);
            tmp.emplace_back(std::move(entry));
        }
        return tmp;
    }

}

void ggg::NSS_kernel::log_request() {
    log("_ > _ _ name=_,uid=_,gid=_,ethernet-address=_,family=_,ip-address=_",
        client_process_id(), this->_database, this->_operation, this->_name, this->_uid,
        this->_gid, this->_ethernet_address, this->_family, this->_ip_address);
}

void ggg::NSS_kernel::log_response() {
    log("_ < _ _ name=_,uid=_,gid=_,ethernet-address=_,family=_,ip-address=_,nbytes=_",
        client_process_id(), this->_database, this->_operation, this->_name, this->_uid,
        this->_gid, this->_ethernet_address, this->_family, this->_ip_address,
        this->_response.position());
}

void ggg::NSS_kernel::run() {
    try {
        do_run();
    } catch (const std::exception& err) {
        result(1);
        this->_response.clear();
    }
}

void ggg::NSS_kernel::do_run() {
    Database entities(Database::File::All, Database::Flag::Read_only);
    auto& resp = this->_response;
    auto* name = this->_name.data();
    switch (this->_database) {
        case Passwd:
            switch (this->_operation) {
                case Get_all: to_bytes<entity>(resp, entities.entities()); break;
                case Get_by_name: to_bytes<entity>(resp, entities.find_entity(name)); break;
                case Get_by_id: to_bytes<entity>(resp, entities.find_entity(this->_uid)); break;
                default: break;
            }
            break;
        case Group:
            switch (this->_operation) {
                case Get_all:
                    to_bytes<group>(resp, entities.groups()); break;
                case Get_by_name: {
                    group gr;
                    if (entities.find_group(name, gr)) {
                        Protocol_traits<group>::write(resp, gr);
                    }
                    break;
                }
                case Get_by_id: {
                    group gr;
                    if (entities.find_group(this->_gid, gr)) {
                        Protocol_traits<group>::write(resp, gr);
                    }
                    break;
                }
                case Init_groups: to_bytes<entity>(resp, entities.find_groups_by_user_name(name)); break;
                default: break;
            }
            break;
        case Shadow:
            switch (this->_operation) {
                case Get_all:
                    to_bytes<account>(resp, entities.accounts()); break;
                case Get_by_name:
                    to_bytes<account>(resp, entities.find_account(name)); break;
                case Get_by_id: break;
                default: break;
            }
            break;
        case Ethers:
            switch (this->_operation) {
                case Get_all: to_bytes<host>(resp, entities.hosts()); break;
                case Get_by_name:
                    to_bytes<host>(resp, entities.find_host(name)); break;
                case Get_by_id:
                    to_bytes<host>(resp, entities.find_host(this->_ethernet_address)); break;
                default: break;
            }
            break;
        case Hosts:
            switch (this->_operation) {
                case Get_all:
                    to_bytes<host_address>(resp, entities.host_addresses());
                    break;
                case Get_by_name:
                    to_bytes<ip_address>(resp, entities.find_ip_address(name, this->_family));
                    break;
                case Get_by_id: {
                    auto st = entities.find_host_name(this->_ip_address);
                    if (st.step() != sqlite::errc::done) {
                        sqlite::cstream cstr(st);
                        std::string name;
                        st.column(0, name);
                        resp.write(name);
                    }
                    break;
                }
                default: break;
            }
            break;
        case Public_keys:
            switch (this->_operation) {
                case Get_by_name:
                    {
                        bool valid = this->_client_credentials.uid == 0;
                        if (!valid) {
                            auto st = entities.find_entity(this->_client_credentials.uid);
                            ggg::entity ent;
                            if (st.step() != sqlite::errc::done) {
                                st >> ent;
                                if (ent.name() == name) { valid = true; }
                            }
                            st.close();
                        }
                        #if !defined(GGG_TEST)
                        if (!valid) { throw std::invalid_argument("permission denied"); }
                        #endif
                    }
                    to_bytes<public_key>(resp, entities.public_keys(name));
                    break;
                default: break;
            }
            break;
        default: break;
    }
}

void ggg::NSS_kernel::read(sys::byte_buffer& buf) {
    //buf.read(this->_result);
    buf.read(this->_database);
    buf.read(this->_operation);
    if (this->_operation == Get_by_name) { buf.read(this->_name); }
    switch (this->_database) {
        case Passwd:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name: break;
                case Get_by_id: buf.read(this->_uid); break;
                default: break;
            }
            break;
        case Group:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name: break;
                case Get_by_id: buf.read(this->_gid); break;
                case Init_groups: buf.read(this->_name); break;
                default: break;
            }
            break;
        case Shadow:
            break;
        case Ethers:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name: break;
                case Get_by_id:
                    buf.read(this->_ethernet_address.data(), this->_ethernet_address.size());
                    break;
                default: break;
            }
            break;
        case Hosts:
            switch (this->_operation) {
                case Get_all:
                    break;
                case Get_by_name:
                    buf.read(this->_family);
                    break;
                case Get_by_id:
                    Protocol_traits<ip_address>::read(buf, this->_ip_address);
                    break;
                default: break;
            }
            break;
        default: break;
    }
    sys::u32 n = 0; buf.read(n);
    if (n > 0) {
        this->_response.resize(n);
        buf.read(this->_response.data(), n);
    }
    this->_response.position(0);
    this->_response.limit(n);
}

void ggg::NSS_kernel::write(sys::byte_buffer& buf) {
    //buf.write(this->_result);
    buf.write(this->_database);
    buf.write(this->_operation);
    if (this->_operation == Get_by_name) { buf.write(this->_name); }
    switch (this->_database) {
        case Passwd:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name: break;
                case Get_by_id: buf.write(this->_uid); break;
                default: break;
            }
            break;
        case Group:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name: break;
                case Get_by_id: buf.write(this->_gid); break;
                case Init_groups: buf.write(this->_name); break;
                default: break;
            }
            break;
        case Shadow:
            break;
        case Ethers:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name: break;
                case Get_by_id:
                    buf.write(this->_ethernet_address.data(), this->_ethernet_address.size());
                    break;
                default: break;
            }
            break;
        case Hosts:
            switch (this->_operation) {
                case Get_all: break;
                case Get_by_name:
                    buf.write(this->_family);
                    break;
                case Get_by_id:
                    Protocol_traits<ip_address>::write(buf, this->_ip_address);
                    break;
                default: break;
            }
            break;
        default: break;
    }
    const auto n = this->_response.position();
    buf.write(static_cast<sys::u32>(n));
    if (n != 0) { buf.write(this->_response.data(), n); }
}

namespace ggg {

    template <> auto
    NSS_kernel::response() -> std::vector<entity> {
        return from_bytes<entity>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<group> {
        return from_bytes<group>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<account> {
        return from_bytes<account>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<host> {
        return from_bytes<host>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<host_address> {
        return from_bytes<host_address>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<ip_address> {
        return from_bytes<ip_address>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<std::string> {
        return from_bytes<std::string>(this->_response);
    }

    template <> auto
    NSS_kernel::response() -> std::vector<public_key> {
        return from_bytes<public_key>(this->_response);
    }

}
