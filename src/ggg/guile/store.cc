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
            store.insert(Guile_traits<entity>::from(obj));
        } else if (type == scm_variable_ref(scm_c_lookup("<account>"))) {
            store.insert(Guile_traits<account>::from(obj));
        } else if (type == scm_variable_ref(scm_c_lookup("<machine>"))) {
            store.insert(Guile_traits<Machine>::from(obj));
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
        } else if (type == scm_variable_ref(scm_c_lookup("<machine>"))) {
            store.erase(Guile_traits<Machine>::from(obj));
        }
        return SCM_UNSPECIFIED;
    }

    SCM
    store_update(SCM store_in, SCM obj) {
        using namespace ggg;
        Store& store = get_store(store_in);
        auto type = scm_class_of(obj);
        if (type == scm_variable_ref(scm_c_lookup("<entity>"))) {
            store.update(Guile_traits<entity>::from(obj));
        } else if (type == scm_variable_ref(scm_c_lookup("<account>"))) {
            store.update(Guile_traits<account>::from(obj));
        }
        return SCM_UNSPECIFIED;
    }

    SCM
    store_select(SCM store_in, SCM obj) {
        using namespace ggg;
        using ::ggg::group;
        Store& store = get_store(store_in);
        auto type = scm_class_of(obj);
        if (type == scm_variable_ref(scm_c_lookup("<entity>"))) {
            auto ent = Guile_traits<entity>::from(obj);
            sqlite::statement st;
            if (ent.has_id()) {
                st = store.find_entity(ent.id());
            } else if (ent.has_name()) {
                st = store.find_entity(ent.name().data());
            }
            if (st.step() != sqlite::errc::done) {
                st >> ent;
                return Guile_traits<entity>::to(ent);
            }
        } else if (type == scm_variable_ref(scm_c_lookup("<account>"))) {
            auto acc = Guile_traits<account>::from(obj);
            auto st = store.find_account(acc.name().data());
            if (st.step() != sqlite::errc::done) {
                st >> acc;
                return Guile_traits<account>::to(acc);
            }
        } else if (type == scm_variable_ref(scm_c_lookup("<group>"))) {
            auto gr = Guile_traits<group>::from(obj);
            bool success = false;
            if (gr.has_id()) {
                success = store.find_group(gr.id(), gr);
            } else if (gr.has_name()) {
                success = store.find_group(gr.name().data(), gr);
            }
            if (success) { return Guile_traits<group>::to(gr); }
        } else if (type == scm_variable_ref(scm_c_lookup("<public-key>"))) {
            auto pk = Guile_traits<public_key>::from(obj);
            auto st = store.public_keys(pk.name().data());
            SCM lst = SCM_EOL;
            if (st.step() != sqlite::errc::done) {
                st >> pk;
                lst = scm_cons(Guile_traits<public_key>::to(pk), lst);
            }
            return lst;
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

    SCM entity_exists(SCM a) {
        using namespace ggg;
        Store store(Store::File::Entities, Store::Flag::Read_only);
        return scm_from_bool(store.has(entity(to_string(a))));
    }

    SCM account_exists(SCM a) {
        using namespace ggg;
        Store store(Store::File::Accounts, Store::Flag::Read_only);
        return scm_from_bool(store.has(account(to_string(a))));
    }

}


bool
ggg::Store::has(const entity& rhs) {
    return has_entity(rhs.name().data());
}

bool
ggg::Store::has(const account& rhs) {
    return find_account(rhs.name().data()).step() != sqlite::errc::done;
}

void
ggg::store_define_procedures() {
    define_procedure("with-transaction", 3, 0, 0, with_transaction);
    define_procedure("add", 2, 0, 0, store_add);
    define_procedure("remove", 2, 0, 0, store_remove);
    define_procedure("update", 2, 0, 0, store_update);
    define_procedure("select", 2, 0, 0, store_select);
    define_procedure("tie", 3, 0, 0, store_tie);
    define_procedure("untie", 3, 0, 0, store_untie);
    define_procedure("attach", 3, 0, 0, store_attach);
    define_procedure("detach", 2, 0, 0, store_detach);
    define_procedure("entity-exists", 1, 0, 0, entity_exists);
    define_procedure("account-exists", 1, 0, 0, account_exists);
    define_procedure("all-accounts", 0, 0, 0, Guile_traits<account>::find);
    define_procedure("all-entities", 0, 0, 0, Guile_traits<entity>::find);
    define_procedure("all-groups", 0, 0, 0, Guile_traits<group>::find);
    define_procedure("all-machines", 0, 0, 0, Guile_traits<Machine>::find);
    define_procedure("remove-all-machines", 0, 0, 0, Guile_traits<Machine>::remove_all);
}
