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

		template <class... Args>
		ControlBlock(Args&&... args) : value(forward<Args>(args)...) {}

		void* get_value() const override { return const_cast<Type*>(&value); }
	};

	STL_NS_IMPL::BaseControlBlock* m_control = nullptr;

	void increase_ref() {
		if (m_control) m_control->counter++;
	}

	void decrease_ref() {
		if (m_control && --m_control->counter <= 0) {
			delete m_control;
		}
	}

	const Type* get_value() const {
		if (m_control) return static_cast<Type*>(m_control->get_value());
		return nullptr;
	}

	Type* get_value() { return const_cast<Type*>(as_const(this)->get_value()); }

	SharedPtr(ControlBlock* block) : m_control(block) {}

	template <class T, class... Args>
	friend SharedPtr<T> make_shared(Args&&... args);

	// friend any other specialization of SharedPtr
	template <class T>
	friend class STL_NS::SharedPtr;

public:
	SharedPtr() {}

	SharedPtr(const SharedPtr& other) : m_control(other.m_control) { increase_ref(); }

	SharedPtr(SharedPtr&& other) : m_control(other.m_control) { other.m_control = nullptr; }

	template <class OtherType>
	requires(!types::is_same<Type, OtherType> && types::convertible_to<OtherType*, Type*>)
	SharedPtr(const SharedPtr<OtherType>& other) : m_control(other.m_control) {
		increase_ref();
	}

	~SharedPtr() { decrease_ref(); }

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

	Type* ptr() { return get_value(); }

	const Type* ptr() const { return get_value(); }

	Type* operator->() { return ptr(); }

	const Type* operator->() const { return ptr(); }

	Type& operator*() { return *ptr(); }

	const Type& operator*() const { return *ptr(); }

	operator bool() const { return ptr(); }

	bool operator==(const SharedPtr& other) const { return m_control == other.m_control; }

	bool operator==(const Type* raw_ptr) const { return ptr() == raw_ptr; }
};

template <class Type, class... Args>
SharedPtr<Type> make_shared(Args&&... args) {
	using Block = typename SharedPtr<Type>::ControlBlock;
	return SharedPtr<Type>(new Block(forward<Args>(args)...));
}

}
