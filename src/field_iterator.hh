#ifndef KEY_ITERATOR_HH
#define KEY_ITERATOR_HH

#include <tuple>
#include <type_traits>

namespace stdx {

	template <class Map, size_t N>
	class field_iterator: public Map::iterator {

	public:
		typedef typename Map::iterator base_iterator;
		typedef typename std::tuple_element<
			N,
			typename base_iterator::value_type
		>::type value_type;
		typedef value_type* pointer;
		typedef const pointer const_pointer;
		typedef value_type& reference;
		typedef const reference const_reference;

		field_iterator() = default;

		field_iterator(base_iterator rhs):
		base_iterator(rhs)
		{}

		field_iterator(const field_iterator& rhs):
		base_iterator(rhs)
		{}

		const_pointer
		operator->() const noexcept {
			return std::get<N>(base_iterator::operator->());
		}

		pointer
		operator->() noexcept {
			return std::get<N>(base_iterator::operator->());
		}

		const_reference
		operator*() const noexcept {
			return std::get<N>(base_iterator::operator*());
		}

		reference
		operator*() noexcept {
			return std::get<N>(base_iterator::operator*());
		}
	};

}

#endif // KEY_ITERATOR_HH
