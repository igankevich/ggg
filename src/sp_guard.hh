#ifndef SP_GUARD_HH
#define SP_GUARD_HH

#include "entity_guard.hh"
#include <shadow.h>

typedef basic_entity_guard<::setspent, ::endspent> sp_guard;

#endif // SP_GUARD_HH


