#include <iostream>

#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/form.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/sec/secure_string.hh>
#include <ggg/config.hh>

int
main() {
	using namespace ggg;
	bool debug = true;
	double min_entropy = 30;
	const char* name = "pi";
	ggg::init_locale();
	form f(name, min_entropy);
	f.set_type(form_type::console);
	bool success, stopped = false;
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
				g.add(ent, ent.origin(), acc);
			}
			success = true;
		} catch (const std::exception& err) {
			success = false;
			std::cerr << std::endl << err.what() << std::endl;
		}
	} while (!success && !stopped);
	if (success) {
		native_message(std::wcout, "Registered successfully!\n");
	}
	std::cout << "press any key to continue..." << std::endl;
	std::cin.get();
	return 0;
}
