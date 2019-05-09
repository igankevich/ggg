#include <netdb.h>
#include <nss.h>

#include <gtest/gtest.h>

#include <ggg/test/clean_database.hh>

struct host_guard {
	host_guard() { ::sethostent(0); }
	~host_guard() { ::endhostent(); }
};

TEST(hosts, empty) {
	{ Clean_database db; }
	host_guard g;
	auto* ent = ::gethostent();
	ASSERT_EQ(nullptr, ent) << ent->h_name;
}
