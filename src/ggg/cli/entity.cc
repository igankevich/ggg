#include <iomanip>
#include <ostream>

#include <ggg/bits/read_field.hh>
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

