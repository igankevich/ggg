#ifndef GGG_CLI_ENTITY_TYPE_HH
#define GGG_CLI_ENTITY_TYPE_HH

namespace ggg {

	enum class Entity_type {Account, Entity, Machine, Message};

	void operator>>(std::string name, Entity_type& type);

	enum class Entity_output_format {Guile, Rec, TSV, Name, NSS};

	void operator>>(std::string name, Entity_output_format& type);

}

#endif // vim:filetype=cpp
