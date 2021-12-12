#pragma once
#include "stl.hpp"

template <auto>
static constexpr bool always_false_v = false;

template <class>
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
