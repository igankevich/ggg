#ifndef CORE_LOCK_HH
#define CORE_LOCK_HH

#include <unistdx/fs/file_mutex>

namespace ggg {

	typedef sys::file_mutex file_mutex_type;

	class file_lock {

	private:
		file_mutex_type _mutex;
		bool _write = false;

	public:

		explicit
		file_lock(bool write=false);

		inline
		~file_lock() { this->unlock(); }

		void lock();
		void unlock();

		file_lock(file_lock&&) = delete;
		file_lock(const file_lock&) = delete;
		file_lock& operator=(const file_lock&) = delete;

	};

}

#endif // vim:filetype=cpp
