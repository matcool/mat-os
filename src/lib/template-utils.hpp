#pragma once

template <auto>
static constexpr bool always_false_v = false;

template <class>
static constexpr bool always_false_t = false;

// TODO: should these be in this file?

template <class R, class... Args>
using FuncPtr = R(*)(Args...);

template <class T>
concept is_destructible = requires { T::~T(); };
