#include <locale>
#include <stdexcept>

#include <ggg/cli/entity_type.hh>

void
ggg::operator>>(std::string name, Entity_type& type) {
	if (name == "entity") { type = Entity_type::Entity; }
	else if (name == "account") { type = Entity_type::Account; }
	else if (name == "machine") { type = Entity_type::Machine; }
	else if (name == "message") { type = Entity_type::Message; }
	else if (name == "public-key") { type = Entity_type::Public_key; }
	else throw std::invalid_argument("unknown entity type");
}

void
ggg::operator>>(std::string name, Format& type) {
    for (auto& ch : name) { ch = std::tolower(ch, std::locale::classic()); }
	if (name == "scm" || name == "guile" || name == "scheme") {
        type = Format::SCM;
    } else if (name == "rec") { type = Format::Rec; }
    else if (name == "tsv") { type = Format::TSV; }
    else if (name == "name") { type = Format::Name; }
    else if (name == "passwd") { type = Format::Passwd; }
    else if (name == "group") { type = Format::Group; }
    else if (name == "shadow") { type = Format::Shadow; }
    else if (name == "hosts") { type = Format::Hosts; }
    else if (name == "ssh") { type = Format::SSH; }
	else throw std::invalid_argument("unknown format");
}
