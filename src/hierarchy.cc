#include "entity_stream.hh"
#include "entity.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>

int main(int argc, char* argv[]) {
	using namespace legion;
	entity_stream<entity> str(sys::path("hierarchy"));
	typedef entity_stream_iterator<entity> iterator;
	std::set<entity> entities;
	entity ent;
	while (str >> ent) {
		entities.emplace(ent);
	}
	std::move(
		entities.begin(),
		entities.end(),
		std::ostream_iterator<entity>(std::cout, "\n")
	);
	return 0;
}
