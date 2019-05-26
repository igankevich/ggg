#include <libguile.h>

#include <ostream>
#include <iomanip>
#include <sstream>

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
SCM
ggg::Guile_traits<ggg::Machine>::insert(SCM args) {
	SCM name = SCM_UNDEFINED, ether = SCM_UNDEFINED, ip = SCM_UNDEFINED;
	scm_c_bind_keyword_arguments("ggg-machine-insert", args,
		scm_t_keyword_arguments_flags{},
		scm_from_utf8_keyword("name"), &name,
		scm_from_utf8_keyword("ethernet-address"), &ether,
		scm_from_utf8_keyword("ip-address"), &ip,
		SCM_UNDEFINED);
	/*
	if (!is_bound(ether)) {
		scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
			scm_from_utf8_string("ethernet-address is unset"));
		return SCM_UNSPECIFIED;
	}
	*/
	Machine m;
	if (is_bound(name)) { m._name =  to_string(name); }
	if (is_bound(ether)) { std::stringstream(to_string(ether)) >> m._ethernet_address; }
	if (is_bound(ip)) { std::stringstream(to_string(ip)) >> m._ip_address; }
	Database db(Database::File::Entities, Database::Flag::Read_write);
	Transaction tr(db);
	db.insert(m);
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
SCM
ggg::Guile_traits<ggg::Machine>::remove(SCM args) {
	SCM name = SCM_UNDEFINED, ether = SCM_UNDEFINED, ip = SCM_UNDEFINED;
	scm_c_bind_keyword_arguments("ggg-machine-delete", args,
		scm_t_keyword_arguments_flags{},
		scm_from_utf8_keyword("name"), &name,
		scm_from_utf8_keyword("ethernet-address"), &ether,
		scm_from_utf8_keyword("ip-address"), &ip,
		SCM_UNDEFINED);
	Database db(Database::File::Entities, Database::Flag::Read_write);
	Transaction tr(db);
	if (is_bound(name)) { db.remove_machine(to_string(name).data()); }
	if (is_bound(ether)) {
		sys::ethernet_address addr;
		std::stringstream(to_string(ether)) >> addr;
		db.remove(addr);
	}
	if (is_bound(ip)) {
		ip_address addr;
		std::stringstream(to_string(ip)) >> addr;
		db.remove(addr);
	}
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
void
ggg::Guile_traits<ggg::Machine>::define_procedures() {
	scm_c_define_gsubr("ggg-machine-insert", 0, 0, 1, (scm_t_subr)&insert);
	scm_c_define_gsubr("ggg-machine-delete", 0, 0, 1, (scm_t_subr)&remove);
	scm_c_define_gsubr("ggg-machine-delete-all", 0, 0, 1, (scm_t_subr)&remove_all);
}

template <> const char* ggg::Guile_traits<ggg::Machine>::name() { return "machine"; }

