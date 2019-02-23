#ifndef GGG_NSS_ETHERENT_HH
#define GGG_NSS_ETHERENT_HH

#include <netinet/ether.h>

struct etherent {
	char* e_name;
	struct ::ether_addr e_addr;
};

#endif // vim:filetype=cpp
