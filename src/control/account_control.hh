#ifndef CONTROL_ACCOUNT_CONTROL_HH
#define CONTROL_ACCOUNT_CONTROL_HH

#include "core/account.hh"

#include <iterator>
#include <istream>

namespace ggg {

	class Account_control {

	public:
		typedef std::istream_iterator<account> iterator;
		typedef std::ostream_iterator<account> oiterator;

	public:
		iterator
		find(const char* user) const;

		inline bool
		exists(const char* user) const {
			return this->find(user) != end();
		}

		void
		erase(const char* user);

		void
		update(const account& acc);

		inline static iterator
		end() {
			return iterator();
		}

	private:
		static inline iterator
		begin(std::istream& in) {
			return iterator(in);
		}

	};

}

#endif // CONTROL_ACCOUNT_CONTROL_HH
