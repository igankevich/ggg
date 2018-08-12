#ifndef CONTROL_ACCOUNT_CONTROL_HH
#define CONTROL_ACCOUNT_CONTROL_HH

#include <istream>
#include <iterator>
#include <unordered_set>

#include <unistdx/fs/file_mode>
#include <unistdx/ipc/identity>

#include <ggg/core/account.hh>
#include <ggg/core/acl.hh>

namespace ggg {

	class account_ctl {

	public:
		typedef char char_type;
		typedef std::unordered_set<account> container_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;

	private:
		container_type _accounts;
		bool _verbose = false;
		/// Group with read access to accounts.
		sys::gid_type _readgid = 0;
		/// Group with write access to accounts.
		sys::gid_type _writegid = 0;

	public:

		inline
		account_ctl() {
			this->open();
		}

		account_ctl(const account_ctl&) = delete;

		account_ctl&
		operator=(const account_ctl&) = delete;

		account_ctl(account_ctl&&) = default;

		~account_ctl() = default;

		void
		open();

		inline const_iterator
		find(const char* user) const {
			return this->_accounts.find(account(user));
		}

		inline bool
		exists(const char* user) const {
			return this->find(user) != end();
		}

		void
		erase(const char* user);

		void
		update(const account& acc);

		void
		update_password(const account& acc);

		void
		add(const account& acc);

		void
		expire(const char* user);

		void
		activate(const char* user);

		void
		expire_password(const char* user);

		inline void
		verbose(bool rhs) noexcept {
			this->_verbose = rhs;
		}

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}

		inline iterator
		begin() {
			return this->_accounts.begin();
		}

		inline iterator
		end() {
			return this->_accounts.end();
		}

		inline const_iterator
		begin() const {
			return this->_accounts.begin();
		}

		inline const_iterator
		end() const {
			return this->_accounts.end();
		}

		inline const_iterator
		cbegin() const {
			return this->_accounts.begin();
		}

		inline const_iterator
		cend() const {
			return this->_accounts.end();
		}

		inline void
		clear() {
			this->_accounts.clear();
		}

		inline void
		set_read_group(sys::gid_type rhs) noexcept {
			this->_readgid = rhs;
		}

		inline void
		set_write_group(sys::gid_type rhs) noexcept {
			this->_writegid = rhs;
		}

	private:

		inline acl::access_control_list
		getperms(sys::uid_type uid) noexcept {
			acl::access_control_list acl(sys::file_mode(uid == 0 ? 0 : 0600));
			if (this->_readgid != 0) {
				acl.add_group(this->_readgid, acl::permission_type::read);
			}
			if (this->_writegid != 0) {
				acl.add_group(this->_writegid, acl::permission_type::write);
			}
			return acl;
		}

	};

}

#endif // CONTROL_ACCOUNT_CONTROL_HH
