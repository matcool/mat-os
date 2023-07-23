#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace STL_NS {

namespace STL_NS_IMPL {
	struct BaseControlBlock {
		i64 counter = 1;
		virtual ~BaseControlBlock() {}
		virtual void* get_value() const = 0;
	};
}

// A reference counted pointer
template <class Type>
class SharedPtr {
	struct ControlBlock : STL_NS_IMPL::BaseControlBlock {
		Type value;
		ControlBlock(Type value) : value(value) {}
		void* get_value() const override {
			return const_cast<Type*>(&value);
		}
	};
	STL_NS_IMPL::BaseControlBlock* m_control = nullptr;
	
	void increase_ref() {
		if (m_control)
			m_control->counter++;
	}
	void decrease_ref() {
        if (m_control && --m_control->counter <= 0) {
            delete m_control;
        }
    }

	const Type* get_value() const {
		if (m_control)
			return static_cast<Type*>(m_control->get_value());
		return nullptr;
	}
	Type* get_value() {
		return const_cast<Type*>(as_const(this)->get_value());
	}

	SharedPtr(ControlBlock* block) : m_control(block) {}
	
	template <class T, class... Args>
	friend SharedPtr<T> make_shared(Args&&... args);
public:
	SharedPtr() {}
	SharedPtr(const SharedPtr& other) : m_control(other.m_control) {
		increase_ref();
	}
	SharedPtr(SharedPtr&& other) : m_control(other.m_control) {
		other.m_control = nullptr;
	}
	template <class OtherType>
	requires (!types::is_same<Type, OtherType> && types::convertible_to<OtherType*, Type*>)
	SharedPtr(const SharedPtr<OtherType>& other) : m_control(other.m_control) {
		increase_ref();
	}
	~SharedPtr() {
		decrease_ref();
	}

	SharedPtr& operator=(const SharedPtr& other) {
		if (this == &other) return *this;
        decrease_ref();
        m_control = other.m_control;
        increase_ref();
        return *this;
	}

	void clear() {
		decrease_ref();
		m_control = nullptr;
	}

	Type* data() { return get_value(); }
	const Type* data() const { return get_value(); }

	Type* operator->() { return data(); }
	const Type* operator->() const { return data(); }

	Type& operator*() { return *data(); }
	const Type& operator*() const { return *data(); }

	operator bool() const { return data(); }
};

template <class Type, class... Args>
SharedPtr<Type> make_shared(Args&&... args) {
    return SharedPtr<Type>(new typename SharedPtr<Type>::ControlBlock(
		Type(forward<Args>(args)...)
	));
}

}
