#include <ggg/guile/store.hh>

bool
ggg::Store::has(const entity& rhs) {
	return contains(rhs.name().data());
}

bool
ggg::Store::has(const account& rhs) {
	return find_account(rhs.name().data()).step() != sqlite::errc::done;
}

bool
ggg::Store::has(const form2& rhs) {
	return find_form(rhs.name().data()).step() != sqlite::errc::done;
}

void
ggg::Store::add(const entity& rhs) {
	insert(rhs);
}

void
ggg::Store::add(const account& rhs) {
	if (!has(entity(rhs.name().data()))) {
		throw std::invalid_argument("bad account");
	}
	insert(rhs);
}

void
ggg::Store::add(const form2& rhs) {
	if (!has(entity(rhs.name().data())) ||
		!has(account(rhs.name().data()))) {
		throw std::invalid_argument("bad form");
	}
	insert(rhs);
}

