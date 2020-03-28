#include <unistdx/base/base64>

#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/unistdx.hh>

namespace {

    SCM scm_encode_base64(SCM data) {
		auto s_data = ggg::to_string(data);
        std::string result;
        result.resize(sys::base64_encoded_size(s_data.size()));
        sys::base64_encode(s_data.data(), s_data.size(), &result[0]);
        return scm_from_utf8_string(result.data());
    }

    SCM scm_decode_base64(SCM data) {
		auto s_data = ggg::to_string(data);
        auto n = sys::base64_max_decoded_size(s_data.size());
        std::string result;
        result.resize(n);
        n = sys::base64_decode(s_data.data(), s_data.size(), &result[0]);
        result.resize(n);
        return scm_from_utf8_string(result.data());
    }

}

void ggg::unistdx_define_procedures() {
    define_procedure("encode-base64", 1, 0, 0, scm_encode_base64);
    define_procedure("decode-base64", 1, 0, 0, scm_decode_base64);
}
