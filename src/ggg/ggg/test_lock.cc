#include "test_lock.hh"

#include <iostream>
#include <mutex>
#include <thread>

#include <unistdx/fs/file_mutex>
#include <unistdx/fs/path>
#include <unistdx/ipc/process>

#include <ggg/config.hh>

namespace {

	void
	test_file_lock(const sys::path& p) {
		typedef sys::file_mutex mutex_type;
		typedef std::lock_guard<mutex_type> lock_type;
		const char* filename = "file_mutex_test.lock";
		mutex_type mtx(filename, 0600);
		if (!mtx) {
			throw std::runtime_error("failed to open file lock");
		}
		sys::process child {
			[filename] () {
				mutex_type mtx2(filename, 0600);
				lock_type lock(mtx2);
				::pause();
				return 0;
			}
		};
		using namespace std::chrono;
		using namespace std::this_thread;
		sleep_for(milliseconds(500));
		if (mtx.try_lock()) {
			throw std::runtime_error("file lock does not work");
		}
		child.terminate();
		child.join();
		if (!mtx.try_lock()) {
			throw std::runtime_error("file lock does not work");
		}
	}

}

void
ggg::Test_lock
::execute() {
	for (const auto& arg : this->args()) {
		sys::path p(arg);
		try {
			test_file_lock(p);
		} catch (...) {
			std::remove(p);
			throw;
		}
	}
}

void
ggg::Test_lock
::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " FILE" << '\n';
}
