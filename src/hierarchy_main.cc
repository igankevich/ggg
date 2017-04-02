#include "hierarchy.hh"

int main(int argc, char* argv[]) {
	legion::Hierarchy h(sys::path("/etc/hierarchy"));
	std::cout << h;
	return 0;
}
