#include <ostream>
#include <iomanip>
#include <sstream>
#include <iostream>

#include <ggg/core/database.hh>
#include <ggg/core/machine.hh>
#include <ggg/guile/guile_traits.hh>

namespace ggg {

    template <>
    ggg::Machine
    Guile_traits<ggg::Machine>::from(SCM obj) {
        Machine m;
        m._name = to_string(slot(obj, "name"));
        std::stringstream(to_string(slot(obj, "ethernet-address"))) >> m._ethernet_address;
        std::stringstream(to_string(slot(obj, "ip-address"))) >> m._ip_address;
        return m;
    }

    template <>
    SCM
    Guile_traits<ggg::Machine>::to(const Machine& m) {
        std::stringstream s1; s1 << m.ethernet_address();
        std::stringstream s2; s2 << m.address();
        return scm_call(
            scm_variable_ref(scm_c_lookup("make")),
            scm_variable_ref(scm_c_lookup("<machine>")),
            scm_from_latin1_keyword("name"),
            scm_from_utf8_string(m.name().data()),
            scm_from_latin1_keyword("ethernet-address"),
            scm_from_utf8_string(s1.str().data()),
            scm_from_latin1_keyword("ip-address"),
            scm_from_utf8_string(s2.str().data()),
            SCM_UNDEFINED
        );
    }

    template <>
    void
    Guile_traits<ggg::Machine>::to_guile(std::ostream& guile, const array_type& objects) {
        if (objects.empty()) { guile << "(list)"; return; }
        std::string indent(2, ' ');
        guile << "(list";
        for (const auto& m : objects) {
            guile << '\n' << indent;
            guile << "(make <machine>\n";
            guile << indent << "      #:name " << escape_string(m.name()) << '\n';
            guile << indent << "      #:ethernet-address \""
                << m.ethernet_address() << "\"\n";
            guile << indent << "      #:ip-address \"" << m.address() << "\")";
        }
        guile << ")\n";
    }

    template <>
    SCM
    Guile_traits<ggg::Machine>::insert(SCM obj) {
        /*
        if (!is_bound(ether)) {
            scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
                scm_from_utf8_string("ethernet-address is unset"));
            return SCM_UNSPECIFIED;
        }
        */
        Database db(Database::File::Entities, Database::Flag::Read_write);
        Transaction tr(db);
        db.insert(from(obj));
        tr.commit();
        return SCM_UNSPECIFIED;
    }

    template <>
    SCM
    Guile_traits<ggg::Machine>::remove(SCM obj) {
        Database db(Database::File::Entities, Database::Flag::Read_write);
        Transaction tr(db);
        db.erase(from(obj));
        tr.commit();
        return SCM_UNSPECIFIED;
    }

    template <>
    SCM
    Guile_traits<ggg::Machine>::remove_all() {
        Database db(Database::File::Entities, Database::Flag::Read_write);
        Transaction tr(db);
        db.remove_all_machines();
        tr.commit();
        return SCM_UNSPECIFIED;
    }

    template <>
    SCM
    Guile_traits<ggg::Machine>::find() {
        Database db(Database::File::Entities, Database::Flag::Read_only);
        auto st = db.machines();
        machine_iterator first(st), last;
        SCM list = SCM_EOL;
        while (first != last) {
            list = scm_append(scm_list_2(list, scm_list_1(to(*first))));
            ++first;
        }
        return list;
    }

    template <>
    void
    Guile_traits<ggg::Machine>::define_procedures() {
        define_procedure("ggg-machine-insert", 1, 0, 0, &insert);
        define_procedure("ggg-machine-delete", 1, 0, 0, &remove);
        define_procedure("ggg-machine-delete-all", 0, 0, 0, &remove_all);
        define_procedure("ggg-machines", 0, 0, 0, &find);
    }

}
