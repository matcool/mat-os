#pragma once
#include "stl.hpp"

template <auto...>
static constexpr bool always_false_v = false;

template <class...>
static constexpr bool always_false_t = false;

template <class F>
using FuncPtr = F*; // lol

template <class T>
concept is_destructible = requires { T::~T(); };

template <class T>
void destroy(T& value) {
	if constexpr (is_destructible<T>)
		value.~T();
}

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

template <class T>
static constexpr bool is_pointer = false;

template <class T>
static constexpr bool is_pointer<T*> = true;

namespace {
	template <class T>
	struct _remove_cv { using type = T; };
	template <class T>
	struct _remove_cv<const T> { using type = T; };
	template <class T>
	struct _remove_cv<volatile T> { using type = T; };
	template <class T>
	struct _remove_cv<const volatile T> { using type = T; };

	template <class T>
	struct _remove_ref { using type = T; };
	template <class T>
	struct _remove_ref<T&> { using type = T; };
	template <class T>
	struct _remove_ref<T&&> { using type = T; };
}

// cv is const/volatile
template <class T>
using remove_cv = typename _remove_cv<T>::type;

template <class T>
using remove_ref = typename _remove_ref<T>::type;

// TODO: maybe put this in another header, stl puts it in <utility>

template <class T>
constexpr T&& forward(remove_ref<T>& value) noexcept {
	return static_cast<T&&>(value);
}

template <class T>
constexpr T&& forward(remove_ref<T>&& value) noexcept {
	return static_cast<T&&>(value);
}

template <class T>
constexpr remove_ref<T>&& move(T&& value) noexcept {
	return static_cast<remove_ref<T>&&>(value);
}

namespace {
	template <class T>
	struct _unsigned_of { using type = T; };
	template <>
	struct _unsigned_of<i8> { using type = u8; };
	template <>
	struct _unsigned_of<i16> { using type = u16; };
	template <>
	struct _unsigned_of<i32> { using type = u32; };
	template <>
	struct _unsigned_of<i64> { using type = u64; };
}

template <integral T>
using unsigned_of = typename _unsigned_of<T>::type;
