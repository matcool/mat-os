#pragma once
#include "stl.hpp"

template <auto...>
static constexpr bool always_false_v = false;

template <class...>
static constexpr bool always_false_t = false;

// TODO: should these be in this file?

template <class R, class... Args>
using FuncPtr = R(*)(Args...);

template <class T>
concept is_destructible = requires { T::~T(); };


template <class T>
static constexpr bool is_integral = false;

// mesmerizing
template <> static constexpr bool is_integral<i8> = true;
template <> static constexpr bool is_integral<u8> = true;
template <> static constexpr bool is_integral<i16> = true;
template <> static constexpr bool is_integral<u16> = true;
template <> static constexpr bool is_integral<i32> = true;
template <> static constexpr bool is_integral<u32> = true;
template <> static constexpr bool is_integral<i64> = true;
template <> static constexpr bool is_integral<u64> = true;

template <class T>
concept integral = is_integral<T>;

namespace {
	template <bool, class T, class F>
	struct _ternary_t { using type = T; };

	template <class T, class F>
	struct _ternary_t<false, T, F> { using type = F; };
}

template <bool v, class T, class F>
using ternary_t = typename _ternary_t<v, T, F>::type;

template <class T, class U>
static constexpr bool is_same = false;

template <class T>
static constexpr bool is_same<T, T> = true;

template <class T, class U, class... Ts>
static constexpr bool is_any_of = is_same<T, U> ? true : is_any_of<T, Ts...>;

template <class T, class U>
static constexpr bool is_any_of<T, U> = is_same<T, U>;
