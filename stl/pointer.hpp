#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace STL_NS {

// A reference counted pointer
template <class Type>
class SharedPtr {
	struct ControlBlock {
		Type value;
		i64 counter = 1;
	};
	ControlBlock* m_control = nullptr;
	
	void increase_ref() {
		if (m_control)
			m_control->counter++;
	}
	void decrease_ref() {
        if (m_control && --m_control->counter <= 0) {
            delete m_control;
        }
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

	Type* data() { return m_control ? &m_control->value : nullptr; }
	const Type* data() const { return m_control ? &m_control->value : nullptr; }

	Type* operator->() { return data(); }
	const Type* operator->() const { return data(); }

	Type& operator*() { return *data(); }
	const Type& operator*() const { return *data(); }

	operator bool() const { return data(); }
};

template <class Type, class... Args>
SharedPtr<Type> make_shared(Args&&... args) {
    return SharedPtr<Type>(new typename SharedPtr<Type>::ControlBlock {
		.value = Type(forward<Args>(args)...)
	});
}

}
