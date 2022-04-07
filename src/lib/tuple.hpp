#pragma once
#include "stl.hpp"
#include "template-utils.hpp"
#include "utils.hpp"

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
		else return m_next.template at<i - 1>();
	};

	template <size_t i>
	const auto& at() const { return at<i>(); };

	template <class... Args>
	auto& operator=(const Tuple<Args...>& args) {
		m_value = args.template at<0>();
		if constexpr (sizeof...(Ts))
			m_next = args.m_next;
		return *this;
	}

	template <class A, class B>
	auto& operator=(const Pair<A, B>& pair) {
		m_value = pair.first;
		this->template at<1>() = pair.second;
		return *this;
	}
};

template <class... Args>
auto ref_tuple(Args&... args) {
	return Tuple<Args&...>{args...};
}
