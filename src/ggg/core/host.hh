#ifndef GGG_CORE_HOST_HH
#define GGG_CORE_HOST_HH

#include <iosfwd>
#include <string>

#include <ggg/nss/etherent.hh>
#include <sqlitex/rstream.hh>
#include <unistdx/net/ethernet_address>

namespace ggg {

	class host {

	private:
		sys::ethernet_address _address;
		std::string _name;

	public:

		host() = default;
		~host() = default;
		host(const host&) = default;
		host& operator=(const host&) = default;
		host(host&&) = default;
		host& operator=(host&&) = default;

		inline const std::string&
		name() const noexcept {
			return this->_name;
		}

		inline std::string&
		name() noexcept {
			return this->_name;
		}

		inline const sys::ethernet_address&
		address() const noexcept {
			return this->_address;
		}

		inline sys::ethernet_address&
		address() noexcept {
			return this->_address;
		}

		inline void
		swap(host& rhs) {
			using std::swap;
			swap(this->_address, rhs._address);
			swap(this->_name, rhs._name);
		}

		inline void
		clear() {
			this->_address = sys::ethernet_address{};
			this->_name.clear();
		}

		inline bool
		operator==(const host& rhs) {
			return
				this->_address == rhs._address &&
				this->_name == rhs._name;
		}

		inline bool
		operator!=(const host& rhs) {
			return this->operator==(rhs);
		}

		inline bool
		operator<(const host& rhs) {
			return
				std::make_tuple(this->_address, this->_name) <
				std::make_tuple(rhs._address, rhs._name);
		}

		friend sqlite::rstream&
		operator>>(sqlite::rstream& in, host& rhs);

	};

	inline void
	swap(host& lhs, host& rhs) {
		lhs.swap(rhs);
	}

	std::ostream&
	operator<<(std::ostream& out, const host& rhs);

	sqlite::rstream&
	operator>>(sqlite::rstream& in, host& rhs);

	inline size_t
	buffer_size(const host& h) noexcept {
		return h.name().size() + 1 + h.address().size();
	}

	void
	copy_to(const host& ent, struct ::etherent* lhs, char* buffer);

}

namespace std {

	template <>
	struct hash<ggg::host>: public hash<string> {

		typedef size_t result_type;
		typedef ggg::host argument_type;

		inline result_type
		operator()(const argument_type& rhs) const noexcept {
			// TODO hash address
			return hash<string>::operator()(rhs.name());
		}

	};

}

#endif // vim:filetype=cpp
