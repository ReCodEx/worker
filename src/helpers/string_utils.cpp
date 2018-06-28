#include "string_utils.h"
#include <cctype>

std::string helpers::random_alphanum_string(std::size_t length)
{
	auto randchar = []() -> char {
		const char charset[] = "0123456789"
							   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							   "abcdefghijklmnopqrstuvwxyz";
		const std::size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

void helpers::filter_non_printable_chars(std::string &text)
{
	text.erase(
		std::remove_if(text.begin(),
			text.end(),
			[](char c) { return !isprint(static_cast<unsigned char>(c)) && !iscntrl(static_cast<unsigned char>(c)); }),
		text.end());
}
