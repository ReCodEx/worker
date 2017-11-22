#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/helpers/string_utils.h"


void test_filter_scenario(std::string expected, std::string text)
{
	helpers::filter_non_printable_chars(text);
	ASSERT_EQ(expected, text);
}

TEST(string_utils_test, filter_non_printable_chars)
{
	test_filter_scenario("", "");
	test_filter_scenario("hello\n\t", "hello\n\t");
	test_filter_scenario("1", "\x31");
	test_filter_scenario("t", "\x74");
	test_filter_scenario("ao", "ação");
	test_filter_scenario("Instalao", "InstalaÃ§Ã£o");
	test_filter_scenario("(^_^)(^_^)", "ヘ(^_^ヘ)(ノ^_^)ノ");
}
