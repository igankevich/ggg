#ifndef GGG_CORE_IP_ADDRESS_HH
#define GGG_CORE_IP_ADDRESS_HH

#include <iosfwd>

#include <sqlitex/rstream.hh>

#include <unistdx/net/family>
#include <unistdx/net/ipv4_address>
#include <unistdx/net/ipv6_address>

namespace ggg {

	class ip_address {

	public:
		typedef char value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;

	private:
		sys::family_type _family = sys::family_type::unspecified;
		union {
			sys::ipv4_address _address4;
			sys::ipv6_address _address6{};
		};

	public:

		ip_address(): _address6{} {}

		inline explicit
		ip_address(sys::family_type family):
		_family{family},
		_address6{} {}

		~ip_address() = default;

		ip_address(const ip_address& rhs);

		ip_address&
		operator=(const ip_address&);

		ip_address(ip_address&&) = default;
		ip_address& operator=(ip_address&&) = default;

		inline sys::family_type
		family() const noexcept {
			return this->_family;
		}

		inline void
		clear() {
			this->_family = sys::family_type::unspecified;
			this->_address6 = sys::ipv6_address{};
		}

		inline size_t
		size() const {
			return (this->_family == sys::family_type::inet
			 ? sizeof(sys::ipv4_address)
			 : sizeof(sys::ipv6_address));
		}

		inline pointer
		data() {
			return reinterpret_cast<pointer>(this->_address4.data());
		}

		inline const_pointer
		data() const {
			return reinterpret_cast<const_pointer>(this->_address4.data());
		}

		friend std::ostream&
		operator<<(std::ostream& out, const ip_address& rhs);

		friend sqlite::rstream&
		operator>>(sqlite::rstream& in, ip_address& rhs);

		friend sqlite::cstream&
		operator>>(sqlite::cstream& in, ip_address& rhs);

		friend bool
		operator==(const ip_address& lhs, const ip_address& rhs);

		friend bool
		operator<(const ip_address& lhs, const ip_address& rhs);

	};

	std::ostream&
	operator<<(std::ostream& out, const ip_address& rhs);

	sqlite::rstream&
	operator>>(sqlite::rstream& in, ip_address& rhs);

	sqlite::cstream&
	operator>>(sqlite::cstream& in, ip_address& rhs);

	bool
	operator==(const ip_address& lhs, const ip_address& rhs);

	inline bool
	operator!=(const ip_address& lhs, const ip_address& rhs) {
		return !operator==(lhs, rhs);
	}

	bool
	operator<(const ip_address& lhs, const ip_address& rhs);

}

#endif // vim:filetype=cpp
