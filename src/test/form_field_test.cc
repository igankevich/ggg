#include "core/form_field.hh"
#include <gtest/gtest.h>
#include <vector>

class FormFieldTest: public ::testing::TestWithParam<const char*> {};

TEST_P(FormFieldTest, Read) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::form_field ff;
	tmp >> ff;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ(123, ff.id());
	EXPECT_EQ("Login", ff.name());
	EXPECT_EQ("[a-z0-9]+", ff.regex());
	EXPECT_EQ("ggg_login", ff.target());
	EXPECT_TRUE(ff.value().empty());
	EXPECT_FALSE(ff.is_constant());
}

INSTANTIATE_TEST_CASE_P(
	ReadWithWhiteSpace,
	FormFieldTest,
	::testing::Values(
		"text:123:Login:ggg_login:[a-z0-9]+",
		" text : 123 : Login : ggg_login : [a-z0-9]+ "
	)
);

TEST(FormFieldTest, ReadEscapedChars) {
	std::stringstream tmp;
	tmp << "text:123:Login:ggg_login:[a-z0-9\\:]+";
	ggg::form_field ff;
	tmp >> ff;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ(123, ff.id());
	EXPECT_EQ("Login", ff.name());
	EXPECT_EQ("[a-z0-9:]+", ff.regex());
	EXPECT_EQ("ggg_login", ff.target());
	EXPECT_TRUE(ff.value().empty());
	EXPECT_FALSE(ff.is_constant());
}

TEST(FormFieldTest, ReadConstant) {
	std::stringstream tmp;
	tmp << "const:ggg_login:qqq";
	ggg::form_field ff;
	tmp >> ff;
	EXPECT_TRUE(tmp.good());
	EXPECT_NE(0, ff.id());
	EXPECT_EQ("", ff.name());
	EXPECT_EQ("", ff.regex());
	EXPECT_EQ("ggg_login", ff.target());
	EXPECT_EQ("qqq", ff.value());
	EXPECT_TRUE(ff.is_constant());
}
