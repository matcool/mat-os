#include "test.hpp"

#include <lib/string.hpp>
#include <lib/vector.hpp>
#include <lib/optional.hpp>
#include <lib/string-utils.hpp>
#include <lib/hash-containers.hpp>
#include <lib/variant.hpp>
#include <lib/result.hpp>

TEST_CASE(string) {
	String str = "hello";
	ASSERT_EQ(str.size(), 5);

	for (int i = 0; i < 10; ++i)
		str.push_back('c');
	ASSERT_EQ(str, "hellocccccccccc"_sv);

	for (int i = 0; i < 20; ++i)
		str.push_back('a');
	ASSERT_EQ(str.size(), 35);

	str.clear();
	ASSERT_EQ(str.size(), 0);

	for (const auto c : str) {
		(void) c;
		ASSERT(false && "String is supposed to be empty!");
	}
}

TEST_CASE(vector) {
	Vector<int> vec;
	ASSERT_EQ(vec.size(), 0);

	for (const auto i : vec) {
		(void) i;
		ASSERT(false && "Vector is supposed to be empty!");
	}

	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);

	ASSERT_EQ(vec.size(), 3);
	ASSERT_EQ(vec[0], 1);

	for (int i = 0; i < 5; ++i)
		vec.push_back(i * 13);

	ASSERT_EQ(vec.size(), 8);
	ASSERT_EQ(vec[5], 26);

	for (auto& i : vec)
		i += 1;

	ASSERT_EQ(vec.size(), 8);
	ASSERT_EQ(vec[5], 27);
}

TEST_CASE(optional) {
	Optional<int> opt = 10;

	ASSERT(opt.has_value());
	ASSERT(opt);
	ASSERT_EQ(*opt, 10);
	ASSERT_EQ(opt.value(), 10);

	opt = {};

	ASSERT(!opt.has_value());
	ASSERT(!opt);
}

TEST_CASE(function) {
	String foo;
	const auto lambda = [&foo]() {
		foo.push_back('a');
	};
	lambda();
	Function<void()> func = lambda;
	func();
	func();
	ASSERT_EQ(foo, "aaa"_sv);
}

TEST_CASE(format) {
	// TODO: maybe have this in the lib
	const auto format = [] (auto... args) {
		String output;
		format_to([&](char c) { output.push_back(c); }, args...);
		return output;
	};
	ASSERT_EQ(format("Hello {}."_sv, "world"_sv), "Hello world."_sv);
	ASSERT_EQ(
		format("Numbers: {} {x} {b} {} {x} {b}"_sv, 135, 135, 135, -30, -30, -30),
		"Numbers: 135 0x87 0b10000111 -30 -0x1E -0b11110"_sv
	);
	ASSERT_EQ(format("Hello {{}} {{hello}} {{ world {} }} {}"_sv, "world"_sv, 42), "Hello {} {hello} { world world } 42"_sv);
}

TEST_CASE(string_utils) {
	ASSERT_EQ(parse_int<u32>("123"_sv), 123);
	ASSERT_EQ(parse_int<u32>("a"_sv, 16), 0xa);
	ASSERT_EQ(parse_int<u32>("0a"_sv, 16), 0xa);
	ASSERT_EQ(parse_int<u32>("20a"_sv, 16), 0x20a);
	ASSERT_EQ(parse_int<u32>("0110110"_sv, 2), 0b0110110);
}

TEST_CASE(hash_map) {
	HashMap<StringView, int> map;
	for ([[maybe_unused]] auto& p : map)
		ASSERT(false && "HashMap should be empty!");

	map["hello"] = 2;
	map["world"_sv] = 3;
	map["hello"_sv] = 10;
	map["foo"] = 2;
	map["bar"] = 4;
	ASSERT_EQ(map.size(), 4);

	for (auto& [key, value] : map)
		++value;
	ASSERT_EQ(map.size(), 4);

	map.remove("foo");
	ASSERT_EQ(map.size(), 3);

	ASSERT_EQ(map["bar"], 5);

	int i = 0, s = 0;
	for (const auto& [key, value] : map) {
		++i;
		s += value;
	}

	ASSERT_EQ(i, 3);
	ASSERT_EQ(s, 20);
}

TEST_CASE(hash_set) {
	HashSet<int> set;
	for ([[maybe_unused]] auto& p : set)
		ASSERT(false && "HashSet should be empty!");

	set.insert(2);
	set.insert(18);
	set.insert(34);
	set.insert(3);
	set.insert(3);

	ASSERT_EQ(set.size(), 4);

	set.remove(18);
	ASSERT_EQ(set.size(), 3);

	set.remove(1);
	ASSERT_EQ(set.size(), 3);

	set.insert(18);
	set.insert(50);
	set.reserve(32);
	ASSERT_EQ(set.size(), 5);
	ASSERT_EQ(set.capacity(), 32);

	for (auto& value : set)
		value *= 2;

	ASSERT_EQ(set.size(), 5);

	int sum = 0;
	for (const auto value : set)
		sum += value;

	ASSERT_EQ(sum, 214);
}

TEST_CASE(variant) {
	Variant<int, const char*, bool> var = 23;
	ASSERT_EQ(var.index(), 0);
	ASSERT_EQ(var.as<0>(), 23);

	var = "hello there!";
	ASSERT_EQ(var.index(), 1);
	ASSERT_EQ(var.as<1>(), "hello there!"_sv);

	var = false;
	ASSERT_EQ(var.index(), 2);
	ASSERT_EQ(var.as<2>(), false);

	ASSERT_EQ(decltype(var)::index_of<bool>, 2);
}

TEST_CASE(result) {
	const auto result_test = [](int x) -> Result<int> {
		if (x < 10) return x;
		else return make_error("The value is too big!"_sv);
	};

	const auto a = result_test(3);
	ASSERT(a.is_ok());
	ASSERT(a);
	ASSERT_EQ(a.ok(), 3);
	ASSERT_EQ(*a, 3);

	auto b = result_test(13);
	ASSERT(b.is_error());
	ASSERT(!b);
	ASSERT_EQ(b.error(), "The value is too big!"_sv);
}
