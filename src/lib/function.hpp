#pragma once
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
	CallableBase<R(Args...)>* m_callable = nullptr;
public:
	template <class T>
	Function(T&& lambda) : m_callable(new Callable<T, R(Args...)>(lambda)) {
	}

	~Function() {
		delete m_callable;
	}

	R operator()(Args... args) {
		m_callable->call(args...);
	}
};