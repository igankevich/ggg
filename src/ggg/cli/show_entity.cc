#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <unistdx/it/intersperse_iterator>

#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>
#include <ggg/cli/object_traits.hh>
#include <ggg/cli/show_entity.hh>

namespace {

//	long
//	to_days(ggg::account::duration d) {
//		using namespace std::chrono;
//		using namespace ggg::chrono;
//		return duration_cast<days>(d).count();
//	}
//
//	void
//	put_days(std::ostream& out, ggg::account::duration d) {
//		if (d > ggg::account::duration::zero()) {
//			out << to_days(d) << " days";
//		} else {
//			out << '-';
//		}
//	}

	void
	put_time_and_date(std::ostream& out, ggg::account::time_point tp) {
		if (tp > ggg::account::time_point(ggg::account::duration::zero())) {
			const char format[] = "%a, %d %b, %Y";
			char status[64];
			std::time_t t = ggg::account::clock_type::to_time_t(tp);
			std::tm* now = std::localtime(&t);
			std::strftime(status, sizeof(status), format, now);
			out << status;
		} else {
			out << '-';
		}
	}

	void
	write_field_values(std::ostream& out, const ggg::entity& rhs) {
		out << "Name: " << rhs.name() << '\n';
		out << "Uid: " << rhs.id() << '\n';
		out << "Gid: " << rhs.gid() << '\n';
		out << "Full name: " << rhs.real_name() << '\n';
		out << "Home: " << rhs.home() << '\n';
		out << "Shell: " << rhs.shell() << '\n';
	}

	void
	write_field_values(std::ostream& out, const ggg::account& acc) {
		out << "Login name: " << acc.login() << '\n';
		ggg::account::time_point now = ggg::account::clock_type::now();
		std::vector<std::string> status;
		if (acc.has_expired(now)) {
			status.push_back("EXPIRED");
		}
		if (acc.password_has_expired(now)) {
			status.push_back("PASSWORD_EXPIRED");
		}
		if (acc.has_been_suspended()) {
			status.push_back("SUSPENDED");
		}
		if (status.empty()) {
			status.push_back("OK");
		}
		out << "Expiration date: ";
		put_time_and_date(out, acc.expire());
		out << '\n';
		out << "Last change date: ";
		put_time_and_date(out, acc.last_change());
		out << '\n';
		out << "Status: ";
		std::copy(
			status.begin(),
			status.end(),
			sys::intersperse_iterator<std::string,char>(out, ' ')
		);
		out << '\n';
	}

	template <class T, class Container>
	void
	show_entities(ggg::Database& db, const Container& args) {
		using namespace ggg;
		typedef Object_traits<T> traits_type;
		std::set<T> cnt;
		traits_type::find(db, args, std::inserter(cnt, cnt.begin()));
		if (cnt.empty()) {
			throw std::runtime_error("not found");
		}
		bool first = false;
		for (const T& ent : cnt) {
			if (!first) {
				first = true;
			} else {
				std::cout << '\n';
			}
			write_field_values(std::cout, ent);
		}
	}

}

void
ggg::Show_entity::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "ae")) != -1) {
		switch (opt) {
			case 'a': this->_type = Type::Account; break;
			case 'e': this->_type = Type::Entity; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_entity::execute() {
	Database db;
	switch (this->_type) {
		case Type::Entity:
			db.open(Database::File::Entities);
			show_entities<entity>(db, this->args());
			break;
		case Type::Account:
			db.open(Database::File::Accounts);
			show_entities<account>(db, this->args());
			break;
	}
}

void
ggg::Show_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " COMMAND" << '\n';
}

