#pragma once
#include "kernel/serial.hpp"
#include "stl.hpp"
#include "template-utils.hpp"

//      |
// thx \/ thx to this
// https://devblogs.microsoft.com/oldnewthing/20200513-00/?p=103745

template <class F>
struct CallableBase;

template <class R, class... Args>
struct CallableBase<R(Args...)> {
	virtual ~CallableBase() {}
	virtual R call(Args... args) = 0;
};

template <class T, class F>
struct Callable;

template <class T, class R, class... Args>
struct Callable<T, R(Args...)> : CallableBase<R(Args...)> {
	T m_value;

	Callable(const T& value) : m_value(value) {}
	Callable(T&& value) : m_value(value) {}

	virtual R call(Args... args) override {
		return m_value(args...);
	}
};

template <class F>
class Function;

template <class R, class... Args>
class Function<R(Args...)> {
	union {
		u8 m_inline_data[24];
		CallableBase<R(Args...)>* m_callable;
	};
	bool m_inline;
public:
	template <class T>
	Function(T&& value) {
		using C = Callable<T, R(Args...)>;
		if (sizeof(C) > 24) {
			m_inline = false;
			m_callable = new C(value);
		} else {
			m_inline = true;
			new (reinterpret_cast<void*>(m_inline_data)) C(value);
		}
	}

	~Function() {
		if (m_inline) {
			using C = CallableBase<R(Args...)>;
			reinterpret_cast<C*>(m_inline_data)->~C();
		} else
			delete m_callable;
	}

	R operator()(Args&&... args) {
		auto callable = m_inline ? reinterpret_cast<CallableBase<R(Args...)>*>(m_inline_data) : m_callable;
		return callable->call(forward<decltype(args)>(args)...);
	}
};
