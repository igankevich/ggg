#include "hierarchy.hh"

int main(int argc, char* argv[]) {
	legion::Hierarchy h(sys::path("hierarchy"));
	h.read();
	h.print();
	return 0;
}
