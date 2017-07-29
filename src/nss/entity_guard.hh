#ifndef ENTITY_GUARD_HH
#define ENTITY_GUARD_HH

template <void Set(), void End()>
class basic_entity_guard {

public:
	basic_entity_guard() { Set(); }
	~basic_entity_guard() { End(); }

	basic_entity_guard(const basic_entity_guard&) = delete;
	basic_entity_guard(basic_entity_guard&&) = delete;
	basic_entity_guard& operator=(const basic_entity_guard&) = delete;
};

#endif // ENTITY_GUARD_HH
