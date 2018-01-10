#include <gtest/gtest.h>
#include <ggg/core/hierarchy.hh>
#include "string_replace.hh"

class HierarchyTest: public ::testing::Test {
protected:
	virtual void
	SetUp() {
		hr.setminuid(0);
	}

	void
	expect_uid_name(sys::uid_type uid, const char* name) {
		auto result = hr.find_by_uid(uid);
		EXPECT_NE(hr.end(), result);
		EXPECT_STREQ(name, result->name().data());
		result = hr.find_by_name(name);
		EXPECT_NE(hr.end(), result);
		EXPECT_EQ(uid, result->id());
	}

	ggg::Hierarchy hr;
};

class HierarchyParamTest:
public HierarchyTest,
public ::testing::WithParamInterface<const char*>
{};

TEST_F(HierarchyTest, DefaultState) {
	EXPECT_EQ(hr.begin(), hr.end());
	EXPECT_EQ(hr.end(), hr.find_by_uid(sys::uid_type(123)));
	EXPECT_EQ(hr.end(), hr.find_by_name("abc"));
}

TEST_F(HierarchyTest, NonExistentRootDirectory) {
	try {
		hr.open("non-existent-directory");
	} catch (std::system_error& err) {
		EXPECT_EQ(
			std::errc::no_such_file_or_directory,
			std::errc(err.code().value())
		);
	}
}

TEST_F(HierarchyTest, EmptyRootDirectory) {
	const char* root = "h-empty";
	ASSERT_EQ(0, run_script(R"(
	rm -rf %
	mkdir -p %
	)", root));
	hr.open(root);
	EXPECT_EQ(hr.begin(), hr.end());
}

TEST_P(HierarchyParamTest, ReadSimple) {
	const char* root = "h-simple";
	ASSERT_EQ(0, run_script(GetParam(), root));
	hr.open(root);
	EXPECT_NE(hr.begin(), hr.end());
	expect_uid_name(7, "halt");
	expect_uid_name(8, "mail");
	expect_uid_name(11, "operator");
	expect_uid_name(12, "games");
}

INSTANTIATE_TEST_CASE_P(
	ReadSimple,
	HierarchyParamTest,
	::testing::Values(
	R"(
	rm -rf %
	mkdir -p %
	echo 'halt:x:7:0:halt:/sbin:/sbin/halt' >> %/file
	echo 'mail:x:8:12:mail:/var/spool/mail:/sbin/nologin' >> %/file
	echo 'operator:x:11:0:operator:/root:/sbin/nologin' >> %/file
	echo 'games:x:12:100:games:/usr/games:/sbin/nologin' >> %/file
	)",
	R"(
	rm -rf %
	mkdir -p %
	mkdir -p %/dir1
	echo 'halt:x:7:0:halt:/sbin:/sbin/halt' >> %/dir1/file
	echo 'mail:x:8:12:mail:/var/spool/mail:/sbin/nologin' >> %/dir1/file
	echo 'operator:x:11:0:operator:/root:/sbin/nologin' >> %/dir1/file
	echo 'games:x:12:100:games:/usr/games:/sbin/nologin' >> %/dir1/file
	)",
	R"(
	rm -rf %
	mkdir -p %
	echo 'halt:x:7:0:halt:/sbin:/sbin/halt' >> %/file1
	echo 'mail:x:8:12:mail:/var/spool/mail:/sbin/nologin' >> %/file2
	echo 'operator:x:11:0:operator:/root:/sbin/nologin' >> %/file3
	echo 'games:x:12:100:games:/usr/games:/sbin/nologin' >> %/file4
	)",
	R"(
	rm -rf %
	mkdir -p %
	mkdir -p %/dir1 %/dir2
	echo 'halt:x:7:0:halt:/sbin:/sbin/halt' >> %/dir1/file1
	echo 'mail:x:8:12:mail:/var/spool/mail:/sbin/nologin' >> %/dir1/file2
	echo 'operator:x:11:0:operator:/root:/sbin/nologin' >> %/dir2/file3
	echo 'games:x:12:100:games:/usr/games:/sbin/nologin' >> %/dir2/file4
	)"
	)
);
