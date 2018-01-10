#include "lock.hh"

#include <sstream>

#include <ggg/config.hh>
#include <unistdx/base/log_message>
#include <unistdx/fs/file_stat>
#include <unistdx/ipc/execute>
#include <unistdx/ipc/process>

ggg::file_lock::file_lock(bool write):
_mutex(
	GGG_LOCK_FILE,
	write ? sys::open_flag::read_write : sys::open_flag::read_only,
	0644
),
_write(write)
{ this->lock(); }

void
ggg::file_lock::lock() {
//	sys::log_message("lck", "locking " GGG_LOCK_FILE);
	sys::file_lock_type tp = this->_write
		? sys::file_lock_type::write_lock
		: sys::file_lock_type::read_lock;
	this->_mutex.lock(tp);
}

void
ggg::file_lock::unlock() {
//	sys::log_message("lck", "unlocking " GGG_LOCK_FILE);
	sys::file_stat st(GGG_GIT_ROOT);
	if (this->_write && st.exists() && st.is_directory()) {
		sys::process p([] () {
			if (-1 == ::chdir(GGG_ROOT)) {
				return 1;
			}
			char host[4096] = {0};
			::gethostname(host, sizeof(host));
			std::stringstream line;
			line << "git add --all && git -c user.name=ggg -c user.email=ggg@";
			line << host << " commit -m 'updated'";
			using namespace sys::this_process;
			return exec("/bin/sh", "-c", line.str());
		});
		sys::proc_status status = p.wait();
		if (status.exited() && status.exit_code() != 0) {
			sys::log_message("lck", "git status=_", status);
		}
	}
	this->_mutex.unlock();
}


