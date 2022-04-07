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
	virtual R call(Args... args) = 0;
	virtual ~CallableBase() {}
	// copies into an uninitialized pointer
	virtual void copy_into(void*) const = 0;
	// allocates a new one
	virtual CallableBase* clone() const = 0;
};

template <class T, class F>
struct Callable;

template <class T, class R, class... Args>
struct Callable<T, R(Args...)> : CallableBase<R(Args...)> {
	T m_value;

	Callable(const T& value) : m_value(value) {}

	virtual R call(Args... args) override {
		return m_value(args...);
	}

	virtual void copy_into(void* ptr) const override {
		new (ptr) Callable(*this);
	}

	virtual Callable* clone() const override {
		return new Callable(*this);
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

	auto get_callable() {
		return m_inline ? static_cast<CallableBase<R(Args...)>*>(reinterpret_cast<void*>(m_inline_data)) : m_callable;
	}

	auto get_callable() const {
		return m_inline ? static_cast<const CallableBase<R(Args...)>*>(reinterpret_cast<const void*>(m_inline_data)) : m_callable;
	}

public:
	Function(const Function& other) : m_inline(other.m_inline) {
		auto* callable = other.get_callable();
		if (m_inline) {
			callable->copy_into(m_inline_data);
		} else {
			m_callable = callable->clone();
		}
	}

	// TODO: move ctor?

	template <class T>
	requires (!is_same<remove_cvref<T>, Function>)
	Function(T&& value) {
		using C = Callable<T, R(Args...)>;
		if constexpr (sizeof(C) > 24) {
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

	R operator()(Args... args) {
		return get_callable()->call(forward<decltype(args)>(args)...);
	}
};
