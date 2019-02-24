#ifndef GGG_INSTANCE_HH
#define GGG_INSTANCE_HH

#include <ggg/core/database.hh>
#include <ggg/nss/entity_traits.hh>
#include <ggg/nss/nss.hh>

namespace ggg {

	template <class T>
	class NSS_database: public Database {

	private:
		typedef entity_traits<T> traits_type;
		typedef typename traits_type::stream_type stream_type;
		typedef typename traits_type::entity_type entity_type;

	private:
		stream_type _stream;
		entity_type _entity;

	public:

		NSS_database() = default;

		inline
		~NSS_database() {
			try {
				this->close();
			} catch (...) {
				// ignore errors for NSS modules
			}
		}

		inline stream_type&
		stream() {
			return this->_stream;
		}

		inline entity_type&
		entity() {
			return this->_entity;
		}

		inline nss_status
		open(Database::File file = Database::File::Entities) {
			nss_status ret;
			try {
				if (this->is_open()) {
					return NSS_STATUS_SUCCESS;
				}
				this->Database::open(file, Database::Flag::Read_only);
				this->_stream = traits_type::all(this);
				this->_stream >> this->_entity;
				ret = NSS_STATUS_SUCCESS;
			} catch (...) {
				ret = NSS_STATUS_UNAVAIL;
			}
			return ret;
		}

		inline nss_status
		close() {
			nss_status ret;
			try {
				this->_stream.close();
				this->Database::close();
				ret = NSS_STATUS_SUCCESS;
			} catch (...) {
				ret = NSS_STATUS_UNAVAIL;
			}
			return ret;
		}

		template <class Entity>
		inline nss_status
		get(Entity* result, char* buffer, size_t buflen, int* errnop) {
			nss_status ret = NSS_STATUS_NOTFOUND;
			int err;
			try {
				if (!this->_stream.good()) {
					ret = NSS_STATUS_NOTFOUND;
					err = ENOENT;
				} else if (buflen < buffer_size(this->_entity)) {
					ret = NSS_STATUS_TRYAGAIN;
					err = ERANGE;
				} else {
					copy_to(this->_entity, result, buffer);
					this->_stream >> this->_entity;
					ret = NSS_STATUS_SUCCESS;
					err = 0;
				}
			} catch (...) {
				ret = NSS_STATUS_UNAVAIL;
				err = ENOENT;
			}
			*errnop = err;
			return ret;
		}

	};

}

#endif // GGG_INSTANCE_HH
