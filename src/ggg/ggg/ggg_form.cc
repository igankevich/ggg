#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/hierarchy.hh>
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/form.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/sec/secure_string.hh>

#include <unistdx/ipc/identity>

namespace ggg {

	void
	register_user(form& f, sys::path origin) {
		const bool debug = true;
		const int max_iterations = 7;
		bool success;
		int i = 0;
		do {
			try {
				entity ent;
				account acc;
				std::tie(ent, acc) = f.input_entity();
				file_lock lock;
				GGG g(GGG_ENT_ROOT, debug);
				lock.unlock();
				bool has_uid = ent.has_id();
				bool has_gid = ent.has_gid();
				if (!has_uid || !has_gid) {
					sys::uid_type id = g.next_uid();
					if (!has_uid) {
						ent.set_uid(id);
					}
					if (!has_gid) {
						ent.set_gid(id);
					}
				}
				{
					file_lock lock(true);
					acc.origin(origin);
					g.add(ent, acc);
				}
				success = true;
			} catch (const std::system_error& err) {
				success = false;
				if (std::errc(err.code().value()) ==
				    std::errc::permission_denied) {
					throw;
				}
			} catch (const std::exception& err) {
				success = false;
				std::cerr << std::endl << err.what() << std::endl;
			}
		} while (!success && ++i < max_iterations);
		if (!success) {
			throw std::runtime_error("reached maximum number of attempts");
		}
		native_message(std::wcout, "\nRegistered successfully!\n");
	}

}

int
main() {
	int ret = 0;
	ggg::init_locale();
	try {
		ggg::form f;
		f.set_type(ggg::form_type::console);
		sys::path origin;
		{
			ggg::file_lock lock;
			ggg::Hierarchy h(GGG_ENT_ROOT);
			auto result = h.find_by_uid(sys::this_process::user());
			if (result == h.end()) {
				throw std::invalid_argument("unable to find form");
			}
			f.open(result->name().data());
			origin = sys::path(GGG_ROOT, "acc", result->name());
		}
		ggg::register_user(f, origin);
	} catch (const std::exception& err) {
		ret = 1;
		std::cerr << err.what() << std::endl;
	}
	if (::isatty(STDIN_FILENO)) {
		std::cout << "press any key to continue..." << std::endl;
		std::cin.get();
	}
	return ret;
}
