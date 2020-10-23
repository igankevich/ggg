#include <iomanip>
#include <ostream>

#include <ggg/core/database.hh>
#include <ggg/core/group.hh>
#include <ggg/guile/guile_traits.hh>

namespace ggg {

    template <>
    ggg::group
    Guile_traits<ggg::group>::from(SCM obj) {
        static_assert(std::is_same<uint32_t,sys::gid_type>::value, "bad guile type");
        group g;
        g._name = to_string(slot(obj, "name"));
        g._gid = scm_to_uint32(slot(obj, "id"));
        auto members = slot(obj, "members");
        auto nmembers = scm_to_int32(scm_length(members));
        for (int i=0; i<nmembers; ++i) {
            g.push(to_string(scm_list_ref(members, scm_from_int32(i))));
        }
        return g;
    }

    template <>
    SCM
    Guile_traits<ggg::group>::to(const group& g) {
        static_assert(std::is_same<uint32_t,sys::uid_type>::value, "bad guile type");
        SCM members = SCM_EOL;
        for (const auto& m : g.members()) {
            members = scm_append(scm_list_2(members, scm_list_1(scm_from_utf8_string(m.data()))));
        }
        return scm_call(
            scm_variable_ref(scm_c_lookup("make")),
            scm_variable_ref(scm_c_lookup("<group>")),
            scm_from_latin1_keyword("name"),
            scm_from_utf8_string(g.name().data()),
            scm_from_latin1_keyword("members"),
            members,
            scm_from_latin1_keyword("id"),
            scm_from_uint32(g.id()),
            SCM_UNDEFINED
        );
    }

    template <>
    SCM
    Guile_traits<ggg::group>::find() {
        Database db(Database::File::Entities, Database::Flag::Read_only);
        const auto& groups = db.groups();
        SCM list = SCM_EOL;
        for (const auto& pair : groups) {
            list = scm_append(scm_list_2(list, scm_list_1(to(pair.second))));
        }
        return list;
    }

}
