#include <ggg/core/account.hh>
#include <ggg/core/days.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/entity_traits.hh>

namespace ggg {

    template <>
    size_t
    buffer_size<account>(const account& a) noexcept {
        return a.name().size() + 1 + a.password().size() + 1;
    }

    template <>
    void
    copy_to<account>(const account& ent, struct ::spwd* lhs, char* buffer) {
        Buffer buf(buffer);
        lhs->sp_namp = buf.write(ent.name());
        lhs->sp_pwdp = buf.write("");
        set_days(lhs->sp_lstchg, ent.last_change());
        set_days(lhs->sp_min, ent.min_change());
        set_days(lhs->sp_max, ent.max_change());
        set_days(lhs->sp_warn, ent.warn_change());
        set_days(lhs->sp_inact, ent.max_inactive());
        set_days(lhs->sp_expire, ent.expire());
    }

}
