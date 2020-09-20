#include "string_utils.h"
#include <cctype>

namespace {

void replace_substring(std::string &data, const std::string &from, const std::string &to)
{
	std::size_t pos = data.find(from);
	while(pos != std::string::npos) {
		data.replace(pos, from.size(), to);
		pos = data.find(from, pos + to.size());
	}
}

void escape_regex(std::string &regex)
{
	replace_substring(regex, "\\", "\\\\");
	replace_substring(regex, "^", "\\^");
	replace_substring(regex, ".", "\\.");
	replace_substring(regex, "$", "\\$");
	replace_substring(regex, "|", "\\|");
	replace_substring(regex, "(", "\\(");
	replace_substring(regex, ")", "\\)");
	replace_substring(regex, "[", "\\[");
	replace_substring(regex, "]", "\\]");
	replace_substring(regex, "*", "\\*");
	replace_substring(regex, "+", "\\+");
	replace_substring(regex, "?", "\\?");
	replace_substring(regex, "/", "\\/");
}

} // namespace

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

std::regex helpers::wildcards_regex(std::string wildcard_pattern)
{
	// Escape all regex special chars
	escape_regex(wildcard_pattern);

	// Convert chars '*?' back to their regex equivalents
	replace_substring(wildcard_pattern, "\\?", ".");
	replace_substring(wildcard_pattern, "\\*", ".*");

	return std::regex(wildcard_pattern);
}
