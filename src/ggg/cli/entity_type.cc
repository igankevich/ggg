#include <stdexcept>

#include <ggg/cli/entity_type.hh>

void
ggg::operator>>(std::string name, Entity_type& type) {
	if (name == "entity") { type = Entity_type::Entity; }
	else if (name == "account") { type = Entity_type::Account; }
	else if (name == "machine") { type = Entity_type::Machine; }
	else if (name == "message") { type = Entity_type::Message; }
	else throw std::invalid_argument("unknown entity type");
}

void
ggg::operator>>(std::string name, Entity_output_format& type) {
	if (name == "scm" || name == "guile" || name == "scheme") {
        type = Entity_output_format::Guile;
    } else if (name == "rec") { type = Entity_output_format::Rec; }
    else if (name == "tsv") { type = Entity_output_format::TSV; }
    else if (name == "name") { type = Entity_output_format::Name; }
    else if (name == "nss") { type = Entity_output_format::NSS; }
	else throw std::invalid_argument("unknown entity output format");
}
