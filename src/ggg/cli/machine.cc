#include <libguile.h>

#include <ostream>
#include <iomanip>
#include <sstream>
#include <iostream>

#include <ggg/cli/guile_traits.hh>
#include <ggg/core/database.hh>
#include <ggg/core/machine.hh>

template <>
char
ggg::Entity_header<ggg::Machine>::delimiter() {
	return ' ';
}

template <>
void
ggg::Entity_header<ggg::Machine>::write_header(
	std::ostream& out,
	width_container width,
	char delim
) {
	if (!width) {
		out << "NAME HWADDR IPADDR\n";
		return;
	}
	const char d[4] = {' ', delim, ' ', 0};
	out << std::left << std::setw(width[0]) << "NAME" << d
		<< std::left << std::setw(width[3]) << "HWADDR" << d
		<< std::left << std::setw(width[4]) << "IPADDR" << '\n';
}

template <>
void
ggg::Entity_header<ggg::Machine>::write_body(
	std::ostream& out,
	const Machine& ent,
	width_container width,
	entity_format fmt,
	char delim
) {
	const char d[4] = {' ', delim, ' ', 0};
	out << std::left << std::setw(width[0]) << ent.name() << d
		<< std::left << std::setw(width[1]) << ent.ethernet_address() << d
		<< std::left << std::setw(width[2]) << ent.address() << '\n';
}

template <>
ggg::Machine
ggg::Guile_traits<ggg::Machine>::from(SCM obj) {
	Machine m;
	m._name = to_string(slot(obj, "name"));
	std::stringstream(to_string(slot(obj, "ethernet-address"))) >> m._ethernet_address;
	std::stringstream(to_string(slot(obj, "ip-address"))) >> m._ip_address;
	return m;
}

template <>
SCM
ggg::Guile_traits<ggg::Machine>::to(const Machine& m) {
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
SCM
ggg::Guile_traits<ggg::Machine>::insert(SCM obj) {
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
ggg::Guile_traits<ggg::Machine>::remove(SCM obj) {
	Database db(Database::File::Entities, Database::Flag::Read_write);
	Transaction tr(db);
	db.remove(from(obj));
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
SCM
ggg::Guile_traits<ggg::Machine>::remove_all() {
	Database db(Database::File::Entities, Database::Flag::Read_write);
	Transaction tr(db);
	db.remove_all_machines();
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
SCM
ggg::Guile_traits<ggg::Machine>::find() {
	Database db(Database::File::Entities, Database::Flag::Read_write);
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
ggg::Guile_traits<ggg::Machine>::define_procedures() {
	scm_c_define_gsubr("ggg-machine-insert", 1, 0, 0, (scm_t_subr)&insert);
	scm_c_define_gsubr("ggg-machine-delete", 1, 0, 0, (scm_t_subr)&remove);
	scm_c_define_gsubr("ggg-machine-delete-all", 0, 0, 0, (scm_t_subr)&remove_all);
	scm_c_define_gsubr("ggg-machines", 0, 0, 0, (scm_t_subr)&find);
}

