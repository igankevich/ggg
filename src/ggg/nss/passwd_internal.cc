#include <ggg/core/entity.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/entity_traits.hh>

namespace ggg {

    template <>
    size_t
    buffer_size<entity>(const entity& rhs) noexcept {
        return rhs.name().size() + 1
               + 1 + 1
               + rhs.description().size() + 1
               + rhs.home().size() + 1
               + rhs.shell().size() + 1;
    }

    template <>
    void
    copy_to<entity,::passwd>(const entity& ent, passwd* lhs, char* buffer) {
        Buffer buf(buffer);
        lhs->pw_name = buf.write(ent.name());
        lhs->pw_passwd = buf.write('x');
        #ifdef __linux__
        lhs->pw_gecos = buf.write(ent.description());
        #endif
        lhs->pw_dir = buf.write(ent.home());
        lhs->pw_shell = buf.write(ent.shell());
        lhs->pw_uid = ent.id();
        lhs->pw_gid = ent.id();
    }

}
