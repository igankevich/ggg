#include <chrono>
#include <ostream>
#include <locale>
#include <ctime>
#include <iomanip>
#include <locale>
#include <ratio>
#include <sstream>
#include <istream>

#include <ggg/core/public_key.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/store.hh>

namespace ggg {

    template <>
    auto
    Guile_traits<public_key>::generate(const string_array& names) -> array_type {
        std::vector<public_key> result;
        result.reserve(names.size());
        for (const auto& name : names) { result.emplace_back(name); }
        return result;
    }

    template <>
    void
    Guile_traits<public_key>::to_guile(std::ostream& out, const array_type& objects) {
        if (objects.empty()) { out << "(list)"; return; }
        out << "(list";
        for (const auto& o : objects) {
            out << "\n  ";
            out << "(make <public-key>\n  ";
            out << "      #:name " << escape_string(o.name()) << "\n  ";
            out << "      #:options " << escape_string(o.options()) << "\n  ";
            out << "      #:type " << escape_string(o.type()) << "\n  ";
            out << "      #:key " << escape_string(o.key()) << "\n  ";
            out << "      #:comment " << escape_string(o.comment()) << ')';
        }
        out << ")\n";
    }

    template <>
    public_key
    Guile_traits<public_key>::from(SCM obj) {
        public_key o;
        o._name = to_string(slot(obj, "name"));
        o._type = to_string(slot(obj, "type"));
        o._key = to_string(slot(obj, "key"));
        o._comment = to_string(slot(obj, "comment"));
        auto s_options = scm_from_latin1_symbol("options");
        if (slot_is_bound(obj, s_options)) {
            o._options = to_string(slot(obj, s_options));
        }
        return o;
    }

    template <>
    SCM
    Guile_traits<ggg::public_key>::to(const public_key& pk) {
        return scm_call(
            scm_variable_ref(scm_c_lookup("make")),
            scm_variable_ref(scm_c_lookup("<public-key>")),
            scm_from_latin1_keyword("name"),
            scm_from_utf8_string(pk.name().data()),
            scm_from_latin1_keyword("options"),
            scm_from_utf8_string(pk.options().data()),
            scm_from_latin1_keyword("comment"),
            scm_from_utf8_string(pk.comment().data()),
            scm_from_latin1_keyword("key"),
            scm_from_utf8_string(pk.key().data()),
            SCM_UNDEFINED
        );
    }

    template <>
    auto
    Guile_traits<public_key>::select(std::string name) -> array_type {
        Store store(Store::File::Accounts, Store::Flag::Read_only);
        auto st = store.public_keys(name.data());
        return array_type(st.begin<public_key>(), st.end<public_key>());
    }

}
