#include "lock.hh"

#include <config.hh>
#include <unistdx/base/log_message>
#include <unistdx/fs/file_stat>
#include <unistdx/ipc/execute>
#include <unistdx/ipc/process>

ggg::file_mutex_type ggg::ggg_mutex(GGG_LOCK_FILE, 0600);

ggg::file_lock::file_lock(bool write):
_write(write) {
	sys::log_message("lck", "locking " GGG_LOCK_FILE);
	ggg_mutex.lock();
}

ggg::file_lock::~file_lock() {
	sys::log_message("lck", "unlocking " GGG_LOCK_FILE);
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
			return execute(
				"/bin/sh",
				"-c",
				line.str()
			);
		});
		sys::proc_status status = p.wait();
		if (status.exited() && status.exit_code() != 0) {
			sys::log_message("lck", "git status=_", status);
		}
	}
	ggg_mutex.unlock();
}


