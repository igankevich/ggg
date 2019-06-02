#ifndef GGG_CLI_ENTITY_TYPE_HH
#define GGG_CLI_ENTITY_TYPE_HH

namespace ggg {

	enum class Entity_type {Account, Entity, Machine, Form};

	void operator>>(std::string name, Entity_type& type);

}

#endif // vim:filetype=cpp
