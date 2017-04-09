#ifndef PW_GUARD_HH
#define PW_GUARD_HH

#include "entity_guard.hh"
#include <pwd.h>

typedef basic_entity_guard<::setpwent, ::endpwent> pw_guard;

#endif // PW_GUARD_HH
