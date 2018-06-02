#include "lock.hh"

#include <sstream>
#include <string>

#include <unistdx/base/log_message>
#include <unistdx/fs/file_stat>
#include <unistdx/fs/path>
#include <unistdx/ipc/execute>
#include <unistdx/ipc/identity>
#include <unistdx/ipc/process>

#include <ggg/config.hh>
#include <ggg/core/fs.hh>

ggg::file_lock
::file_lock(bool write):
_mutex(),
_write(write) {
	const sys::uid_type uid = sys::this_process::user();
	const sys::open_flag flags =
		write ? sys::open_flag::read_write : sys::open_flag::read_only;
	if (uid == 0) {
		this->_mutex.open(GGG_LOCK_FILE, flags, 0644);
	} else {
		this->_mutex.open(
			sys::path(GGG_LOCK_DIR, std::to_string(uid)),
			flags,
			0600
		);
	}
	this->lock();
}

void
ggg::file_lock
::lock() {
//	sys::log_message("lck", "locking " GGG_LOCK_FILE);
	const auto tp = this->_write
	                ? sys::file_lock_type::write_lock
					: sys::file_lock_type::read_lock;
	this->_mutex.lock(tp);
}

void
ggg::file_lock
::unlock() {
//	sys::log_message("lck", "unlocking " GGG_LOCK_FILE);
	if (this->_write) {
		try {
			sys::file_stat st(GGG_GIT_ROOT);
			if (st.is_directory() && can_write(GGG_GIT_ROOT)) {
				sys::process p {
					[] () {
						if (-1 == ::chdir(GGG_ROOT)) {
							return 1;
						}
						char host[4096] = {0};
						::gethostname(host, sizeof(host));
						const size_t n = std::char_traits<char>::length(host);
						const sys::uid_type uid = sys::this_process::user();
						sys::argstream line;
						line.append("/bin/sh");
						line.append("-c");
						line << "git add --all && git -c user.name="
						     << uid << " -c user.email=" << uid << "@'";
						for (size_t i=0; i<n; ++i) {
							const char ch = host[i];
							line << (!std::isalnum(ch) ? '_' : ch);
						}
						line << "' commit -m 'updated'" << '\0';
						return sys::this_process::execute(line);
					}
				};
				sys::proc_status status = p.wait();
				if (status.exited() && status.exit_code() != 0) {
					sys::log_message("lck", "git status=_", status);
				}
			}
		} catch (const sys::bad_call& err) {
			if (err.errc() != std::errc::no_such_file_or_directory) {
				throw;
			}
		}
	}
	this->_mutex.unlock();
}
