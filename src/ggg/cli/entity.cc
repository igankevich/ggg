#include <iomanip>
#include <ostream>
#include <sstream>

#include <ggg/bits/read_field.hh>
#include <ggg/cli/guile_traits.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>

template <>
char
ggg::Entity_header<ggg::entity>::delimiter() {
	return ':';
}

template <>
void
ggg::Entity_header<ggg::entity>::write_header(
	std::ostream& out,
	width_container width,
	char delim
) {
	if (!width) {
		out << "USERNAME:unused:ID:unused:REALNAME:HOME:SHELL\n";
		return;
	}
	const char d[4] = {
		' ',
		delim,
		delim == ' ' ? char(0) : ' ',
		0
	};
	out << std::left << std::setw(width[0]) << "USERNAME" << d
		<< std::right << std::setw(width[2]) << "ID" << d
		<< std::left << std::setw(width[4]) << "REALNAME" << d
		<< std::left << std::setw(width[5]) << "HOME" << d
		<< std::left << std::setw(width[6]) << "SHELL" << '\n';
}

template <>
void
ggg::Entity_header<ggg::entity>::write_body(
	std::ostream& out,
	const entity& ent,
	const width_container width,
	entity_format fmt,
	char delim
) {
	const char d[4] = {
		' ',
		delim,
		delim == ' ' ? char(0) : ' ',
		0
	};
	out << std::left << std::setw(width[0]) << ent.name() << d;
	if (fmt == entity_format::batch) {
		out << std::right << std::setw(width[2]) << ent.id() << d;
	}
	out << std::left << std::setw(width[4]) << ent.real_name() << d
	    << std::left << std::setw(width[5]) << ent.home() << d
	    << std::left << std::setw(width[6]) << ent.shell() << '\n';
}

template <>
std::istream&
ggg::Entity_header<ggg::entity>::read_body(
	std::istream& in,
	entity& ent,
	entity_format fmt
) {
	std::istream::sentry s(in);
	if (!s) {
		return in;
	}
	ent.clear();
	if (fmt == entity_format::batch) {
		bits::read_all_fields(
			in,
			entity::delimiter,
			ent._name,
			ent._uid,
			ent._realname,
			ent._homedir,
			ent._shell
		);
		ent._gid = ent._uid;
	} else {
		bits::read_all_fields(
			in,
			entity::delimiter,
			ent._name,
			ent._realname,
			ent._homedir,
			ent._shell
		);
	}
	if (in.eof()) {
		in.clear();
	}
	return in;
}

template <>
ggg::entity
ggg::Guile_traits<ggg::entity>::from(SCM obj) {
	static_assert(std::is_same<scm_t_uint32,sys::uid_type>::value, "bad guile type");
	entity ent;
	ent._name = to_string(slot(obj, "name"));
	ent._realname = to_string(slot(obj, "real-name"));
	ent._homedir = to_string(slot(obj, "home-directory"));
	if (ent._homedir.empty()) { ent._homedir = "/home/" + ent._name; }
	ent._shell = to_string(slot(obj, "shell"));
	ent._uid = scm_to_uint32(slot(obj, "id"));
	ent._gid = ent._uid;
	return ent;
}

template <>
SCM
ggg::Guile_traits<ggg::entity>::to(const entity& ent) {
	static_assert(std::is_same<scm_t_uint32,sys::uid_type>::value, "bad guile type");
	return scm_call(
		scm_variable_ref(scm_c_lookup("make")),
		scm_variable_ref(scm_c_lookup("<entity>")),
		scm_from_latin1_keyword("name"),
		scm_from_utf8_string(ent.name().data()),
		scm_from_latin1_keyword("real-name"),
		scm_from_utf8_string(ent.real_name().data()),
		scm_from_latin1_keyword("home-directory"),
		scm_from_utf8_string(ent.home().data()),
		scm_from_latin1_keyword("shell"),
		scm_from_utf8_string(ent.shell().data()),
		scm_from_latin1_keyword("id"),
		scm_from_uint32(ent.id()),
		SCM_UNDEFINED
	);
}

template <>
std::string
ggg::Guile_traits<ggg::entity>::to_guile(const entity& ent) {
	std::stringstream guile;
	guile << "(make <entity>\n";
	guile << "      #:name " << escape_string(ent.name()) << '\n';
	guile << "      #:real-name " << escape_string(ent.real_name()) << '\n';
	guile << "      #:home-directory " << escape_string(ent.home()) << '\n';
	guile << "      #:shell " << escape_string(ent.shell()) << '\n';
	guile << "      #:id " << ent.id() << ")\n";
	return guile.str();
}

template <>
SCM
ggg::Guile_traits<ggg::entity>::insert(SCM obj) {
	Database db(Database::File::Entities, Database::Flag::Read_write);
	Transaction tr(db);
	db.insert(from(obj));
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
SCM
ggg::Guile_traits<ggg::entity>::remove(SCM obj) {
	Database db(Database::File::Entities, Database::Flag::Read_write);
	Transaction tr(db);
	db.erase(from(obj));
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
SCM
ggg::Guile_traits<ggg::entity>::find() {
	Database db(Database::File::Entities, Database::Flag::Read_only);
	auto st = db.users();
	user_iterator first(st), last;
	SCM list = SCM_EOL;
	while (first != last) {
		list = scm_append(scm_list_2(list, scm_list_1(to(*first))));
		++first;
	}
	return list;
}

template <>
void
ggg::Guile_traits<ggg::entity>::define_procedures() {
	scm_c_define_gsubr("ggg-entity-insert", 1, 0, 0, (scm_t_subr)&insert);
	scm_c_define_gsubr("ggg-entity-delete", 1, 0, 0, (scm_t_subr)&remove);
	scm_c_define_gsubr("ggg-entities", 0, 0, 0, (scm_t_subr)&find);
	scm_c_define_gsubr("ggg-users", 0, 0, 0, (scm_t_subr)&find);
}