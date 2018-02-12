#ifndef GGG_CORE_SETS_HH
#define GGG_CORE_SETS_HH

namespace ggg {

	template <class S1, class S2>
	inline void
	set_union(S1& lhs, const S2& rhs) {
		lhs.insert(rhs.begin(), rhs.end());
	}

	template <class S1, class S2>
	inline void
	set_difference(S1& lhs, const S2& rhs) {
		for (const auto& elem : rhs) {
			lhs.erase(elem);
		}
	}

}

#endif // vim:filetype=cpp
