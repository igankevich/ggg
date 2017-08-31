#ifndef CORE_LOCK_HH
#define CORE_LOCK_HH

#include <unistdx/fs/file_mutex>

namespace ggg {

	typedef sys::file_mutex<sys::write_lock> file_mutex_type;
	extern file_mutex_type ggg_mutex;

	class file_lock {

	private:
		bool _write = false;

	public:

		inline explicit
		file_lock(bool write=false):
		_write(write)
		{ this->lock(); }

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
