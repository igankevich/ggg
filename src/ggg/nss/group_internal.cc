#include <ggg/core/group.hh>
#include <ggg/nss/buffer.hh>
#include <ggg/nss/entity_traits.hh>

namespace ggg {

    template <>
    size_t
    buffer_size<group>(const group& gr) noexcept {
        size_t sum = 0;
        sum += gr.name().size() + 1;
        sum += 1; // empty password
        for (const std::string& member : gr.members()) {
            sum += member.size() + 1;
        }
        sum += sizeof(Pointer)*(gr.members().size() + 1);
        sum += alignof(Pointer) - 1;
        return sum;
    }

    template <>
    void
    copy_to<group>(const group& gr, struct ::group* lhs, char* buffer) {
        Buffer buf(buffer);
        lhs->gr_name = buf.write(gr.name());
        lhs->gr_passwd = buf.write("");
        auto n = gr.members().size();
        std::unique_ptr<Pointer[]> mem(new Pointer[n]);
        size_t i = 0;
        for (const auto& member : gr.members()) {
            mem[i].ptr = buf.write(member.data());
            ++i;
        }
        lhs->gr_mem = buf.write(mem.get(), n);
        lhs->gr_gid = gr.id();
    }

}
