#ifndef CORE_LOCK_HH
#define CORE_LOCK_HH

#include <unistdx/fs/file_mutex>

namespace ggg {

	typedef sys::file_mutex<sys::write_lock> file_mutex_type;
	extern file_mutex_type ggg_mutex;

	class file_lock {

	private:
		file_mutex_type _mtx;
		bool _write = false;

	public:

		file_lock(bool write=false);
		~file_lock();

		file_lock(file_lock&&) = delete;
		file_lock(const file_lock&) = delete;
		file_lock& operator=(const file_lock&) = delete;

	};

}

#endif // vim:filetype=cpp
