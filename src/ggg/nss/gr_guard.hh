#ifndef GR_GUARD_HH
#define GR_GUARD_HH

#include "entity_guard.hh"
#include <grp.h>

typedef basic_entity_guard<::setgrent, ::endgrent> gr_guard;

#endif // GR_GUARD_HH

