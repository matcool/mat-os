#pragma once

#ifndef MAT_OS
#include <string.h>
#include <iostream>
#include <new>
#else
#include "common.hpp"
#endif

#include <lib/string.hpp>
#include <lib/format.hpp>
#include <lib/vector.hpp>
#include <lib/function.hpp>
#include <lib/utils.hpp>

template <class... Args>
void print(Args&&... args) {
#ifndef MAT_OS
	format_to([](char c) { std::cout.put(c); }, args...);
#endif
}

inline Vector<Pair<StringView, Function<void()>>> test_cases;
inline void add_test_case(const StringView name, const Function<void()>& func) {
	test_cases.push_back({ name, func });
}

#define TEST_CASE(name) \
	void test_case_##name(); \
	namespace { \
		bool _dumb_test_case_##name = (add_test_case(#name, &test_case_##name), 0); \
	} \
	void test_case_##name() \

#ifdef MAT_OS
void exit(int);
#endif

inline void assert(const bool cond, const StringView code, const size_t line, const StringView file_name) {
	if (!cond) {
		print("\x1b[31;1mAssertion failed!\x1b[0m\n"
		      "{}:{} assert({});\n", file_name, line, code);
		exit(1);
	}
}

template <class T, class U>
void assert_eq(const bool cond, T&& a, U&& b, const StringView x, const StringView y, const size_t line, const StringView file_name) {
	if (!cond) {
		print("\x1b[31;1mAssertion failed!\x1b[0m '{}' did not equal '{}'\n"
		      "{}:{} assert({} == {});\n", a, b, file_name, line, x, y);
		exit(1);
	}
}

#define ASSERT(x) assert(static_cast<bool>(x), #x, __LINE__, __FILE__)
#define ASSERT_EQ(x, y) assert_eq((x) == (y), x, y, #x, #y, __LINE__, __FILE__)

int main() {
	for (auto& [name, func] : test_cases) {
		print("\x1b[38;5;49m[info]\x1b[0m running test case \x1b[38;5;222m{}\x1b[0m\n", name);
		func();
	}
	return 0;
}
