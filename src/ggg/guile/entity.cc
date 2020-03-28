#include <iomanip>
#include <ostream>
#include <iostream>
#include <sstream>

#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/store.hh>

namespace {

}

namespace ggg {

    template <>
    ggg::entity
    Guile_traits<ggg::entity>::from(SCM obj) {
        static_assert(std::is_same<scm_t_uint32,sys::uid_type>::value, "bad guile type");
        auto s_description = scm_from_latin1_symbol("description");
        auto s_home = scm_from_latin1_symbol("home");
        auto s_shell = scm_from_latin1_symbol("shell");
        auto s_id = scm_from_latin1_symbol("id");
        entity ent;
        ent._name = to_string(slot(obj, "name"));
        if (slot_is_bound(obj, s_description)) {
            ent._description = to_string(slot(obj, s_description));
        }
        if (slot_is_bound(obj, s_home)) {
            ent._homedir = to_string(slot(obj, s_home));
        } else {
            ent._homedir = GGG_DEFAULT_HOME_PREFIX "/" + ent._name;
        }
        if (slot_is_bound(obj, s_shell)) {
            ent._shell = to_string(slot(obj, s_shell));
        } else {
            ent._shell = GGG_DEFAULT_SHELL;
        }
        if (slot_is_bound(obj, s_id)) {
            ent._id = scm_to_uint32(slot(obj, "id"));
        }
        return ent;
    }

    template <>
    SCM
    Guile_traits<ggg::entity>::to(const entity& ent) {
        static_assert(std::is_same<scm_t_uint32,sys::uid_type>::value, "bad guile type");
        return scm_call(
            scm_variable_ref(scm_c_lookup("make")),
            scm_variable_ref(scm_c_lookup("<entity>")),
            scm_from_latin1_keyword("name"),
            scm_from_utf8_string(ent.name().data()),
            scm_from_latin1_keyword("description"),
            scm_from_utf8_string(ent.description().data()),
            scm_from_latin1_keyword("home"),
            scm_from_utf8_string(ent.home().data()),
            scm_from_latin1_keyword("shell"),
            scm_from_utf8_string(ent.shell().data()),
            scm_from_latin1_keyword("id"),
            scm_from_uint32(ent.id()),
            SCM_UNDEFINED
        );
    }

    template <>
    void
    Guile_traits<ggg::entity>::to_guile(std::ostream& guile, const array_type& objects) {
        if (objects.empty()) { guile << "(list)"; return; }
        std::string indent(2, ' ');
        guile << "(list";
        for (const auto& ent : objects) {
            guile << '\n' << indent;
            guile << "(make <entity>\n";
            guile << indent << "      #:name " << escape_string(ent.name()) << '\n';
            guile << indent << "      #:description " << escape_string(ent.description()) << '\n';
            guile << indent << "      #:home " << escape_string(ent.home()) << '\n';
            guile << indent << "      #:shell " << escape_string(ent.shell());
            if (ent.has_id()) {
                guile << '\n';
                guile << indent << "      #:id " << ent.id() << ")";
            } else {
                guile << ')';
            }
        }
        guile << ")\n";
    }

    template <>
    SCM
    Guile_traits<ggg::entity>::insert0(SCM obj) {
        Store store(Store::File::Entities, Store::Flag::Read_write);
        store.insert(from(obj));
        return SCM_UNSPECIFIED;
    }

    template <>
    SCM
    Guile_traits<ggg::entity>::remove(SCM obj) {
        Store store(Store::File::Entities, Store::Flag::Read_write);
        Transaction tr(store);
        store.erase(from(obj));
        tr.commit();
        return SCM_UNSPECIFIED;
    }

    template <>
    auto
    Guile_traits<ggg::entity>::select(std::string name) -> array_type {
        Store store(Store::File::Entities, Store::Flag::Read_only);
        auto st = store.find_entity(name.data());
        return array_type(user_iterator(st), user_iterator());
    }

    template <>
    SCM
    Guile_traits<ggg::entity>::find() {
        Store store(Store::File::Entities, Store::Flag::Read_only);
        auto st = store.users();
        user_iterator first(st), last;
        SCM list = SCM_EOL;
        while (first != last) {
            list = scm_append(scm_list_2(list, scm_list_1(to(*first))));
            ++first;
        }
        return list;
    }

    template <>
    auto
    Guile_traits<ggg::entity>::generate(const string_array& names) -> array_type {
        std::vector<entity> result;
        result.reserve(names.size());
        for (const auto& name : names) {
            result.emplace_back(name.data());
            auto& tmp = result.back();
            tmp.home(sys::path(GGG_DEFAULT_HOME_PREFIX, name));
            tmp.shell(GGG_DEFAULT_SHELL);
        }
        return result;
    }

}
