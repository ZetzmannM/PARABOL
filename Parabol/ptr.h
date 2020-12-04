#pragma once
#include <memory>
#include <utility>


/// @brief Just an alias, to make is clear, that this pointer is only used to PASS data from one place to another
template<typename T>
using pass_ptr = std::unique_ptr<T>;

/// @brief A class similar to unique_ptr but copyable, in the sense, that deep-copies are performed when this object is being copied (!)
template<
	typename T>
struct wrap_ptr {
private:
	pass_ptr<T> data;
public:

	wrap_ptr();
	wrap_ptr(wrap_ptr<T>&&)
	wrap_ptr(const wrap_ptr<T>&);
	
	wrap_ptr(pass_ptr<T>&&);
	
	wrap_ptr& operator=(const wrap_ptr<T>&);
	wrap_ptr& operator=(wrap_ptr<T>&&);

	wrap_ptr& operator=(pass_ptr<T>&&);
	
	/// @brief Whether this wrap_ptr holds a valid handle
	operator bool() const { return data; }

	T* operator->();
	const T* operator->() const;

	/// @brief DOES NOT RELEASE OWNERSHIP!
	/// @return returns managed pointer
	T* get();

	/// @brief DOES NOT RELEASE OWNERSHIP!
	/// @return returns managed pointer
	const T* get() const;

private:
	/// @brief Copies from input
	/// @param ref copy source
	void _cpy(const wrap_ptr& ref);
};

template<typename T>
wrap_ptr<T>::wrap_ptr() : data{ nullptr } { }
template<typename T>
wrap_ptr<T>::wrap_ptr(pass_ptr<T>&& ref) {
	this->data = std::move(ref);
}
template<typename T>
wrap_ptr<T>::wrap_ptr(wrap_ptr<T>&& ref) {
	this->data = std::move(ref.data);
}
template<typename T>
wrap_ptr<T>::wrap_ptr(const wrap_ptr<T>& ref) {
	_cpy(ref);
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(const wrap_ptr<T>& ref) {
	_cpy(ref);
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(wrap_ptr<T>&& ref) {
	this->data = std::move(ref.data);
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(pass_ptr<T>&& ref) {
	this->data = std::move(ref);
}
template<typename T>
T* wrap_ptr<T>::operator->() {
	return this->data.operator->();
}
template<typename T>
const T* wrap_ptr<T>::operator->() const {
	return this->data.operator->();
}
template<typename T>
void wrap_ptr<T>::_cpy(const wrap_ptr& ref) {
	this->data = pass_ptr<T>(new T(*ref.get()));
}
template<typename T>
T* wrap_ptr<T>::get() {
	return this->data.get();
}
template<typename T>
const T* wrap_ptr<T>::get() const {
	return this->data.get();
}
