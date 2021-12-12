#pragma once

template <auto>
static constexpr bool always_false_v = false;

template <class>
static constexpr bool always_false_t = false;