#pragma once
#include "stl.hpp"
#include "template-utils.hpp"

template <class...>
struct Tuple;

template <>
struct Tuple<> {
	// should this exist?
	template <size_t>
	using type_at = void;

	constexpr size_t size() const { return 0; }
};

template <class T, class... Ts>
struct Tuple<T, Ts...> {
private:
	T m_value;
	Tuple<Ts...> m_next;
public:
	Tuple(T&& value, Ts&& ...rest) : m_value(value), m_next(rest...) {}

	template <size_t i>
	using type_at = ternary_t<i == 0, T, typename decltype(m_next)::template type_at<i - 1>>;

	constexpr size_t size() const { return sizeof...(Ts) + 1; }

	template <size_t i>
	auto& at() {
		if constexpr (i == 0) return m_value;
		return m_next.template at<i - 1>();
	};

	template <size_t i>
	auto at() const { return at<i>(); };
};
