#pragma once
#include "stl.hpp"
#include "template-utils.hpp"
#include "math.hpp"
#include "tuple.hpp"


template <class... Types>
class Variant {
	static_assert(sizeof...(Types), "Variant needs at least one type");

public:
	static constexpr size_t type_count = sizeof...(Types);

	using IndexType = smallest_type_for_indexing<type_count>;

private:
	u8 m_data[max(sizeof(Types)...)];
	IndexType m_index;

	template <size_t N, class T, class U, class... Ts>
	static constexpr IndexType _index_of = is_same<T, U> ? N : _index_of<N + 1, T, Ts...>;

	template <size_t N, class T, class U>
	static constexpr IndexType _index_of<N, T, U> = is_same<T, U> ? N : -1;

	using TypesTuple = Tuple<Types...>;
	using FirstType = typename TypesTuple::template type_at<0>;

public:
	template <class T>
	static constexpr IndexType index_of = _index_of<0, T, Types...>;

	template <class T>
	requires is_any_of<T, Types...>
	Variant(T&& value) {
		m_index = index_of<T>;
		new (m_data) T(forward<T>(value));
	}

	Variant() requires (is_default_constructible<FirstType>) : Variant(FirstType()) {

	}

	auto index() const { return m_index; }

	template <class T>
	requires is_any_of<T, Types...>
	bool contains() const { return m_index == index_of<T>; }

	template <size_t N>
	auto& as() {
		// TODO: assert that m_index == N

		using T = typename TypesTuple::template type_at<N>;

		// this is rly funny
		union bar {
			T foo;
		};
		return reinterpret_cast<bar*>(&m_data)->foo;
	}

private:
	template <size_t N>
	void _destroy() {
		if (m_index == N) {
			auto& value = as<N>();
			::destroy(value);
		} else if constexpr (N < type_count - 1) {
			_destroy<N + 1>();
		}
	}


	template <class T, size_t N>
	struct _find_suitable_type {
		static_assert(N < type_count, "Unable to find suitable type");
		using U = typename TypesTuple::template type_at<N>;
		static auto calc() {
			if constexpr (requires (T a) { static_cast<U>(a); })
				return TypeIdentity<U>{};
			else
				return _find_suitable_type<T, N + 1>::calc();
		}
	};

public:
	~Variant() {
		_destroy<0>();
	}

	template <class T>
	auto& operator=(T&& value) {
		if constexpr (!is_any_of<T, Types...>) {
			return operator= <typename decltype(_find_suitable_type<T, 0>::calc())::type>(value);
		} else {
			static constexpr auto i = index_of<T>;
			if (m_index == i) {
				as<i>() = forward<T>(value);
			} else {
				_destroy<0>();
				m_index = i;
				new (m_data) T(forward<T>(value));
			}
			return *this;
		}
	}
};
