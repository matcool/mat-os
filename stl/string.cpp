#include "string.hpp"

namespace STL_NS {

bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

bool is_ascii_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

char to_ascii_uppercase(char c) {
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 'A';
	return c;
}

char to_ascii_lowercase(char c) {
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 'a';
	return c;
}

}