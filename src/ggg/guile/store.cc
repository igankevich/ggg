#include <ggg/guile/store.hh>
#include <ggg/guile/guile_traits.hh>

#include <iostream>

namespace {

	SCM
	with_transaction(SCM file, SCM flags, SCM procedure) {
		using namespace ggg;
		try {
			Store store(Store::File(scm_to_uint64(file)), Store::Flag(scm_to_uint64(flags)));
			Transaction tr(store);
			scm_call(procedure, from_pointer(&store), SCM_UNDEFINED);
			tr.commit();
		} catch (const std::exception& err) {
			guile_throw(err);
		}
		return SCM_UNSPECIFIED;
	}

	SCM
	store_add(SCM store_in, SCM obj) {
		using namespace ggg;
		auto ptr = scm_to_intptr_t(store_in);
		Store& store = *reinterpret_cast<Store*>(ptr);
		auto type = scm_class_of(obj);
		if (type == scm_variable_ref(scm_c_lookup("<entity>"))) {
			store.add(Guile_traits<entity>::from(obj));
		} else if (type == scm_variable_ref(scm_c_lookup("<account>"))) {
			store.add(Guile_traits<account>::from(obj));
		}
		return SCM_UNSPECIFIED;
	}

}


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

void
ggg::store_define_procedures() {
	define_procedure("with-transaction", 3, 0, 0, with_transaction);
	define_procedure("store-add", 2, 0, 0, store_add);
}

