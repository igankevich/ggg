#include <iomanip>
#include <ostream>
#include <sstream>

#include <ggg/bits/read_field.hh>
#include <ggg/cli/guile_traits.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>

namespace {

	SCM entity_tie(SCM a, SCM b) {
		using namespace ggg;
		if (scm_is_integer(a) && scm_is_integer(b)) {
			Database db(Database::File::Entities, Database::Flag::Read_write);
			Transaction tr(db);
			db.tie(scm_to_uint32(a), scm_to_uint32(b));
			tr.commit();
		} else if (is_string(a) && is_string(b)) {
			Database db(Database::File::Entities, Database::Flag::Read_write);
			Transaction tr(db);
			db.tie(to_string(a).data(), to_string(b).data());
			tr.commit();
		} else {
			scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
				scm_from_utf8_string("ggg-entity-tie needs either two IDs or two names"));
			return SCM_UNSPECIFIED;
		}
		return SCM_UNSPECIFIED;
	}

	SCM entity_untie(SCM a, SCM b) {
		using namespace ggg;
		if (!is_string(a) || (is_bound(b) && !is_string(b))) {
			scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
				scm_from_utf8_string("ggg-entity-untie needs either one or two names"));
			return SCM_UNSPECIFIED;
		}
		Database db(Database::File::Entities, Database::Flag::Read_write);
		Transaction tr(db);
		auto child = to_string(a);
		if (is_bound(b)) { db.untie(child.data(), to_string(b).data()); }
		else { db.untie(child.data()); }
		tr.commit();
		return SCM_UNSPECIFIED;
	}

	SCM entity_attach(SCM a, SCM b) {
		using namespace ggg;
		if (scm_is_integer(a) && scm_is_integer(b)) {
			Database db(Database::File::Entities, Database::Flag::Read_write);
			Transaction tr(db);
			db.attach(scm_to_uint32(a), scm_to_uint32(b));
			tr.commit();
		} else if (is_string(a) && is_string(b)) {
			Database db(Database::File::Entities, Database::Flag::Read_write);
			Transaction tr(db);
			db.attach(to_string(a).data(), to_string(b).data());
			tr.commit();
		} else {
			scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
				scm_from_utf8_string("ggg-entity-attach needs either two IDs or two names"));
			return SCM_UNSPECIFIED;
		}
		return SCM_UNSPECIFIED;
	}

	SCM entity_detach(SCM a, SCM b) {
		using namespace ggg;
		if (!is_string(a)) {
			scm_throw(scm_from_utf8_symbol("ggg-invalid-argument"),
				scm_from_utf8_string("ggg-entity-detach needs string argument"));
			return SCM_UNSPECIFIED;
		}
		Database db(Database::File::Entities, Database::Flag::Read_write);
		Transaction tr(db);
		db.detach(to_string(a).data());
		tr.commit();
		return SCM_UNSPECIFIED;
	}

}

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
	auto s_real_name = scm_from_latin1_symbol("real-name");
	auto s_home = scm_from_latin1_symbol("home-directory");
	auto s_shell = scm_from_latin1_symbol("shell");
	auto s_id = scm_from_latin1_symbol("id");
	entity ent;
	ent._name = to_string(slot(obj, "name"));
	if (slot_is_bound(obj, s_real_name)) {
		ent._realname = to_string(slot(obj, s_real_name));
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
		ent._uid = scm_to_uint32(slot(obj, "id"));
		ent._gid = ent._uid;
	}
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
void
ggg::Guile_traits<ggg::entity>::to_guile(std::ostream& guile, const array_type& objects) {
	if (objects.empty()) { guile << "(list)"; return; }
	std::string indent(2, ' ');
	guile << "(list";
	for (const auto& ent : objects) {
		guile << '\n' << indent;
		guile << "(make <entity>\n";
		guile << indent << "      #:name " << escape_string(ent.name()) << '\n';
		guile << indent << "      #:real-name " << escape_string(ent.real_name()) << '\n';
		guile << indent << "      #:home-directory " << escape_string(ent.home()) << '\n';
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
auto
ggg::Guile_traits<ggg::entity>::select(std::string name) -> array_type {
	Database db(Database::File::Entities, Database::Flag::Read_only);
	auto st = db.find_entity(name.data());
	return array_type(user_iterator(st), user_iterator());
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
auto
ggg::Guile_traits<ggg::entity>::generate(const string_array& names) -> array_type {
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

template <>
void
ggg::Guile_traits<ggg::entity>::define_procedures() {
	scm_c_define_gsubr("ggg-entity-insert", 1, 0, 0, (scm_t_subr)&insert);
	scm_c_define_gsubr("ggg-entity-delete", 1, 0, 0, (scm_t_subr)&remove);
	scm_c_define_gsubr("ggg-entities", 0, 0, 0, (scm_t_subr)&find);
	scm_c_define_gsubr("ggg-users", 0, 0, 0, (scm_t_subr)&find);
	scm_c_define_gsubr("ggg-entity-tie", 2, 0, 0, (scm_t_subr)&entity_tie);
	scm_c_define_gsubr("ggg-entity-untie", 1, 1, 0, (scm_t_subr)&entity_untie);
	scm_c_define_gsubr("ggg-entity-attach", 2, 0, 0, (scm_t_subr)&entity_attach);
	scm_c_define_gsubr("ggg-entity-detach", 1, 0, 0, (scm_t_subr)&entity_detach);
}
