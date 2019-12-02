#include <ggg/guile/store.hh>
#include <ggg/guile/guile_traits.hh>

#include <iostream>

namespace {

    inline ggg::Store&
    get_store(SCM store_in) {
		auto ptr = scm_to_intptr_t(store_in);
		return *reinterpret_cast<ggg::Store*>(ptr);
    }

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
		Store& store = get_store(store_in);
		auto type = scm_class_of(obj);
		if (type == scm_variable_ref(scm_c_lookup("<entity>"))) {
			store.add(Guile_traits<entity>::from(obj));
		} else if (type == scm_variable_ref(scm_c_lookup("<account>"))) {
			store.add(Guile_traits<account>::from(obj));
		}
		return SCM_UNSPECIFIED;
	}

	SCM
	store_remove(SCM store_in, SCM obj) {
		using namespace ggg;
		Store& store = get_store(store_in);
		auto type = scm_class_of(obj);
		if (type == scm_variable_ref(scm_c_lookup("<entity>"))) {
			store.erase(Guile_traits<entity>::from(obj));
		} else if (type == scm_variable_ref(scm_c_lookup("<account>"))) {
			store.erase(Guile_traits<account>::from(obj));
		}
		return SCM_UNSPECIFIED;
	}

	SCM
    store_tie(SCM store_in, SCM a, SCM b) {
		using namespace ggg;
		Store& store = get_store(store_in);
		if (scm_is_integer(a) && scm_is_integer(b)) {
			store.tie(scm_to_uint32(a), scm_to_uint32(b));
		} else if (is_string(a) && is_string(b)) {
			store.tie(to_string(a).data(), to_string(b).data());
		} else {
            guile_throw("tie needs either two IDs or two names");
		}
		return SCM_UNSPECIFIED;
	}

	SCM
    store_untie(SCM store_in, SCM a, SCM b) {
		using namespace ggg;
		if (!is_string(a) || (is_bound(b) && !is_string(b))) {
            guile_throw("untie needs either one or two names");
			return SCM_UNSPECIFIED;
		}
		Store& store = get_store(store_in);
		auto child = to_string(a);
		if (is_bound(b)) { store.untie(child.data(), to_string(b).data()); }
		else { store.untie(child.data()); }
		return SCM_UNSPECIFIED;
	}

	SCM
    store_attach(SCM store_in, SCM a, SCM b) {
		using namespace ggg;
		Store& store = get_store(store_in);
		if (scm_is_integer(a) && scm_is_integer(b)) {
			store.attach(scm_to_uint32(a), scm_to_uint32(b));
		} else if (is_string(a) && is_string(b)) {
			store.attach(to_string(a).data(), to_string(b).data());
		} else {
            guile_throw("attach needs either two IDs or two names");
		}
		return SCM_UNSPECIFIED;
	}

	SCM
    store_detach(SCM store_in, SCM a) {
		using namespace ggg;
		if (!is_string(a)) {
            guile_throw("detach needs string argument");
			return SCM_UNSPECIFIED;
		}
		Store& store = get_store(store_in);
		store.detach(to_string(a).data());
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
ggg::Store::add(const public_key& rhs) {
	insert(rhs);
}

void
ggg::store_define_procedures() {
	define_procedure("with-transaction", 3, 0, 0, with_transaction);
	define_procedure("add", 2, 0, 0, store_add);
	define_procedure("remove", 2, 0, 0, store_remove);
	define_procedure("tie", 3, 0, 0, store_tie);
	define_procedure("untie", 3, 0, 0, store_untie);
	define_procedure("attach", 3, 0, 0, store_attach);
	define_procedure("detach", 2, 0, 0, store_detach);
}

