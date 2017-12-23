#ifndef CONTROL_ACCOUNT_CONTROL_HH
#define CONTROL_ACCOUNT_CONTROL_HH

#include "core/account.hh"

#include <iterator>
#include <istream>
#include <functional>

namespace ggg {

	class account_ctl {

	public:
		typedef std::istream_iterator<account> iterator;
		typedef std::ostream_iterator<account> oiterator;
		typedef std::function<void(account&)> update_account;
		typedef std::function<void(const account&)> process_account;

	private:
		bool _verbose = false;

	public:
		iterator
		find(const char* user) const;

		inline bool
		exists(const char* user) const {
			return this->find(user) != end();
		}

		void
		for_each(process_account func) const;

		void
		erase(const char* user);

		void
		update(const account& acc);

		void
		update(const char* acc, update_account func);

		account
		generate(const char* user);

		void
		add(const account& acc);

		inline static iterator
		end() {
			return iterator();
		}

		inline void
		verbose(bool rhs) noexcept {
			this->_verbose = rhs;
		}

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}

	private:
		static inline iterator
		begin(std::istream& in) {
			return iterator(in);
		}

	};

}

#endif // CONTROL_ACCOUNT_CONTROL_HH
