#pragma once
#include "stl.hpp"

template <class T>
class OwnPtr {
	T* m_ptr;
public:
	OwnPtr(const OwnPtr&) = delete;
	OwnPtr(T* ptr) : m_ptr(ptr) {}

	~OwnPtr() { delete m_ptr; }

	const T* data() const { return m_ptr; }
	T* data() { return m_ptr; }

	const T* operator->() const { return m_ptr; }
	T* operator->() { return m_ptr; }

	const T& operator*() const { return *m_ptr; }
	T& operator*() { return *m_ptr; }
};
