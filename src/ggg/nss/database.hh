#ifndef GGG_NSS_DATABASE_HH
#define GGG_NSS_DATABASE_HH

#include <netdb.h>

#include <ggg/nss/entity_traits.hh>
#include <ggg/nss/nss.hh>
#include <ggg/proto/protocol.hh>
#include <ggg/proto/selection.hh>

namespace ggg {

    template <class T, NSS_kernel::DB db>
    class NSS_response {

    public:
        using value_type = T;
        using container_type = std::vector<T>;
        using iterator = typename container_type::iterator;

    private:
        container_type _entities;
        iterator _first, _last;

    public:
        inline value_type& entity() { return *this->_first; }
        inline bool empty() const { return this->_first == this->_last; }

        inline nss_status
        open() {
            nss_status ret = NSS_STATUS_SUCCESS;
            try {
                NSS_kernel kernel(db, NSS_kernel::Get_all);
                Client_protocol proto(GGG_CLIENT_CONF);
                proto.process(&kernel, Protocol::Command::NSS_kernel);
                this->_entities = kernel.response<value_type>();
                this->_first = this->_entities.begin();
                this->_last = this->_entities.end();
            } catch (const std::exception& ex) {
                sys::message("%s: %s", to_string(db), ex.what());
                ret = NSS_STATUS_UNAVAIL;
            } catch (...) {
                sys::message("%s: unknown error", to_string(db));
                ret = NSS_STATUS_UNAVAIL;
            }
            return ret;
        }

        inline nss_status close() {
            this->_entities.clear();
            this->_first = this->_entities.begin();
            this->_last = this->_entities.end();
            return NSS_STATUS_SUCCESS;
        }

        template <class Entity>
        inline nss_status
        get(Entity* result, char* buffer, size_t buflen, int* errnop) {
            nss_status ret = NSS_STATUS_NOTFOUND;
            int err;
            try {
                if (this->_first == this->_last) {
                    ret = NSS_STATUS_NOTFOUND;
                    err = ENOENT;
                } else if (buflen < buffer_size(*this->_first)) {
                    ret = NSS_STATUS_TRYAGAIN;
                    err = ERANGE;
                } else {
                    copy_to(*this->_first, result, buffer);
                    ++this->_first;
                    ret = NSS_STATUS_SUCCESS;
                    err = 0;
                }
            } catch (const std::exception& ex) {
                sys::message("%s: %s", to_string(db), ex.what());
                ret = NSS_STATUS_UNAVAIL;
                err = ENOENT;
            } catch (...) {
                sys::message("%s: unknown error", to_string(db));
                ret = NSS_STATUS_UNAVAIL;
                err = ENOENT;
            }
            *errnop = err;
            return ret;
        }

        template <class Entity>
        inline nss_status
        get(Entity* result, char* buffer, size_t buflen, int* errnop, int* h_errnop) {
            nss_status ret = NSS_STATUS_NOTFOUND;
            int err, h_err;
            try {
                if (this->_first == this->_last) {
                    ret = NSS_STATUS_NOTFOUND;
                    err = ENOENT;
                    h_err = HOST_NOT_FOUND;
                } else if (buflen < buffer_size(*this->_first)) {
                    ret = NSS_STATUS_TRYAGAIN;
                    err = ERANGE;
                    h_err = 0;
                } else {
                    copy_to(*this->_first, result, buffer);
                    ++this->_first;
                    ret = NSS_STATUS_SUCCESS;
                    err = 0;
                    h_err = 0;
                }
            } catch (const std::exception& ex) {
                sys::message("%s: %s", to_string(db), ex.what());
                ret = NSS_STATUS_UNAVAIL;
                err = ENOENT;
            } catch (...) {
                sys::message("%s: unknown error", to_string(db));
                ret = NSS_STATUS_UNAVAIL;
                err = ENOENT;
                h_err = NO_RECOVERY;
            }
            *errnop = err;
            *h_errnop = h_err;
            return ret;
        }

    };

}

#endif // vim:filetype=cpp
