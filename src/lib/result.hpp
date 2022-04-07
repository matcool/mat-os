#pragma once
#include "stl.hpp"
#include "template-utils.hpp"
#include "string.hpp"

template <class T, class E = StringView>
class [[nodiscard]] Result {
protected:
	union {
		T m_value_ok;
		E m_value_error;
	};
	// TODO: reconsider const here as it disallows move and assignment
	// although i have no plan of having mutable Result
	const bool m_success;
public:
	Result() = delete;
	Result(const Result&) = delete;
	constexpr Result(T&& value) : m_value_ok(forward<T>(value)), m_success(true) {}
	constexpr Result(const T& value) : m_value_ok(value), m_success(true) {}
	constexpr Result(bool, E&& value) : m_value_error(forward<E>(value)), m_success(false) {}
	constexpr Result(bool, const E& value) : m_value_error(value), m_success(false) {}

	constexpr bool is_ok() const { return m_success; }
	constexpr bool is_error() const { return !m_success; }

	explicit constexpr operator bool() const { return m_success; }

	[[nodiscard]] constexpr T& ok() { return m_value_ok; }
	[[nodiscard]] constexpr const T& ok() const { return m_value_ok; }
	[[nodiscard]] constexpr E& error() { return m_value_error; }
	[[nodiscard]] constexpr const E& error() const { return m_value_error; }

	// to be consistent with Optional
	[[nodiscard]] constexpr T& value() { return m_value_ok; }
	[[nodiscard]] constexpr const T& value() const { return m_value_ok; }

	[[nodiscard]] constexpr T& operator*() { return m_value_ok; }
	[[nodiscard]] constexpr const T& operator*() const { return m_value_ok; }
	[[nodiscard]] constexpr T* operator->() { return &m_value_ok; }
	[[nodiscard]] constexpr const T* operator->() const { return &m_value_ok; }

	constexpr ~Result() {
		if (m_success)
			m_value_ok.~T();
		else
			m_value_error.~E();
	}
};

namespace {
	struct VoidResult {};
}

template <class E>
class [[nodiscard]] Result<void, E> : public Result<VoidResult, E> {
public:
	Result() : Result(VoidResult{}) {}
	using Result<VoidResult, E>::Result;
};


namespace {
	template <class E>
	struct ResultError {
		E m_value;

		ResultError(const E& value) : m_value(value) {}

		template <class T, class U>
		operator Result<T, U>() {
			return Result<T, U>(false, m_value);
		}
	};
}

template <class E>
auto make_error(E&& err) {
	return ResultError<E>(err);
}
