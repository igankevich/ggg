#include <stdexcept>

#include <ggg/cli/entity_type.hh>

void
ggg::operator>>(std::string name, Entity_type& type) {
	if (name == "entity") { type = Entity_type::Entity; }
	else if (name == "account") { type = Entity_type::Account; }
	else if (name == "machine") { type = Entity_type::Machine; }
	else if (name == "form") { type = Entity_type::Form; }
	else throw std::invalid_argument("unknown entity type");
}
