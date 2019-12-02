#ifndef GGG_CLI_ENTITY_TYPE_HH
#define GGG_CLI_ENTITY_TYPE_HH

namespace ggg {

	enum class Entity_type {Account, Entity, Machine, Message, Public_key};

	void operator>>(std::string name, Entity_type& type);

	enum class Format {SCM, Rec, TSV, Name, NSS, SSH};

	void operator>>(std::string name, Format& type);

}

#endif // vim:filetype=cpp
