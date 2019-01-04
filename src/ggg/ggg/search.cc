#include <ggg/ggg/search.hh>

namespace {

void regular_expression_search(
	sqlite3_context* context,
	int,
	sqlite3_value** value
) {
	ggg::Search* ptr =
		static_cast<ggg::Search*>(sqlite3_user_data(context));
	const char* field =
		reinterpret_cast<const char*>(sqlite3_value_text(*value));
	::sqlite3_result_int(context, ptr->search(field));
}

}

void
ggg::Search::create_function() {
	this->db->db()->scalar_function(
		regular_expression_search,
		"search",
		1,
		sqlite::encoding::utf8,
		true,
		this
	);
}

void
ggg::Search::delete_function() {
	this->db->db()->remove_function("search", 1, sqlite::encoding::utf8);
}

