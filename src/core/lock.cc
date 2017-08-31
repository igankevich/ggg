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
			using namespace sys::this_process;
			return execute(
				"/bin/sh",
				"-c",
				"git add --all && git -c user.name=ggg -c user.email=ggg@ggg commit -m 'updated'"
			);
		});
		sys::proc_status status = p.wait();
		if (status.exited() && status.exit_code() != 0) {
			sys::log_message("lck", "git status=_", status);
		}
	}
	ggg_mutex.unlock();
}


