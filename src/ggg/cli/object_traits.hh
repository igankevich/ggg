#ifndef GGG_OBJECT_TRAITS_HH
#define GGG_OBJECT_TRAITS_HH

#include <string>
#include <ostream>
#include <fstream>
#include <stdexcept>

#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/entity_format.hh>

namespace ggg {

	template <class T>
	struct Object_traits;

	template <>
	struct Object_traits<ggg::entity> {

		inline static const std::string&
		name(const ggg::entity& rhs) noexcept {
			return rhs.name();
		}

		inline static bool
		eq(const ggg::entity& lhs, const ggg::entity& rhs) {
			return (lhs.has_id() && lhs.id() == rhs.id()) || lhs.name() == rhs.name();
		}

		inline static bool
		equal_names(const ggg::entity& lhs, const ggg::entity& rhs) {
			return lhs.name() == rhs.name();
		}

		inline static std::ostream&
		print(std::ostream& out, const ggg::entity& rhs) {
			return out << rhs.name() << ':' << rhs.id();
		}

		inline static char
		delimiter() noexcept {
			return ggg::entity::delimiter;
		}

		template <class Container, class Result>
		inline static void
		find(ggg::Database& db, const Container& args, Result result) {
			db.find_users(args.begin(), args.end(), result);
		}

	};

	template <>
	struct Object_traits<ggg::account> {

		inline static const char*
		name(const ggg::account& rhs) noexcept {
			return rhs.login().data();
		}

		inline static bool
		eq(const ggg::account& lhs, const ggg::account& rhs) {
			return lhs.login() == rhs.login();
		}

		inline static bool
		equal_names(const ggg::account& lhs, const ggg::account& rhs) {
			return lhs.name() == rhs.name();
		}

		inline static std::ostream&
		print(std::ostream& out, const ggg::account& rhs) {
			return out << rhs.login();
		}

		inline static char
		delimiter() noexcept {
			return ggg::account::delimiter;
		}

		template <class Container, class Result>
		inline static void
		find(ggg::Database& db, const Container& args, Result result) {
			db.find_accounts(args.begin(), args.end(), result);
		}

	};

	template <class T, class Result>
	void
	read_objects(
		const std::string& filename,
		entity_format format,
		Result result,
		std::string msg
	) {
		typedef Entity_header<T> traits_type;
		std::ifstream in;
		try {
			in.exceptions(std::ios::badbit);
			in.imbue(std::locale::classic());
			in.open(filename, std::ios_base::in);
			T obj;
			while (traits_type::read_body(in, obj, format)) {
				*result++ = obj;
			}
		} catch (...) {
			if (!in.eof()) {
				throw std::system_error(std::io_errc::stream, msg);
			}
		}
	}

	template <class Container, class BinOp>
	void
	check_duplicates(const Container& objs, BinOp op) {
		const size_t n = objs.size();
		for (size_t i=0; i<n; ++i) {
			const auto& lhs = objs[i];
			for (size_t j=i+1; j<n; ++j) {
				const auto& rhs = objs[j];
				if (op(lhs, rhs)) {
					throw std::invalid_argument("duplicate names/ids");
				}
			}
		}
	}

}

#endif // GGG_OBJECT_TRAITS_HH
