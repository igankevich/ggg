#include "hierarchy.hh"

int main(int argc, char* argv[]) {
	ggg::Hierarchy h(sys::path("/etc/hierarchy"));
	std::cout << h;
	return 0;
}
