#ifndef GROUP_HH
#define GROUP_HH

#include <string>
#include <unordered_set>
#include <unistd.h>
#include <grp.h>

namespace sys {
	typedef ::gid_t gid_type;
}

namespace legion {

	class group {

		std::string _name;
		std::string _password;
		sys::gid_type _gid;
		mutable std::unordered_set<std::string> _members;

	public:
		explicit
		group(
			const std::string& name,
			const std::string& password,
			sys::gid_type gid
		):
		_name(name),
		_password(password),
		_gid(gid)
		{}

		explicit
		group(const std::string& name):
		_name(name)
		{}

		group() = default;
		group(const group& rhs) = default;
		group(group&& rhs) = default;
		~group() = default;

		group&
		operator=(const group& rhs) = default;

		void
		copy_to(struct ::group* lhs, char* buffer) const;

		size_t
		buffer_size() const noexcept;

		inline bool
		has_id() const noexcept {
			return this->_gid != sys::gid_type(-1);
		}

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

		sys::gid_type
		id() const noexcept {
			return _gid;
		}

		const std::string&
		name() const noexcept {
			return _name;
		}

		const std::string&
		password() const noexcept {
			return _password;
		}

		const std::unordered_set<std::string>&
		members() const noexcept {
			return _members;
		}

		inline void
		push(const std::string& member) const {
			_members.emplace(member);
		}

		inline void
		erase(const std::string& member) const {
			_members.erase(member);
		}

		inline void
		clear() const {
			_members.clear();
		}
	};

	std::ostream&
	operator<<(std::ostream& out, const group& rhs);

}

#endif // GROUP_HH
