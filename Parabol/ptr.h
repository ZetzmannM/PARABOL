#pragma once
#include <memory>
#include <utility>
#include <string>
#include <vector>

/// @brief Just an alias, to make is clear, that this pointer is only used to PASS data from one place to another
template<typename T>
using pass_ptr = std::unique_ptr<T>;

/// @brief A class similar to unique_ptr but copyable, in the sense, that deep-copies are performed when this object is being copied (!)
template< typename T >
struct wrap_ptr {
private:
	pass_ptr<T> data;

public:

	wrap_ptr();
	wrap_ptr(T* vkInst);
	wrap_ptr(wrap_ptr<T>&&) noexcept;
	wrap_ptr(const wrap_ptr<T>&);
	
	wrap_ptr(pass_ptr<T>&&);
	
	wrap_ptr& operator=(const wrap_ptr<T>&);
	wrap_ptr& operator=(wrap_ptr<T>&&) noexcept;

	wrap_ptr& operator=(pass_ptr<T>&&) noexcept;
	wrap_ptr& operator=(T*& vkInst);
	

	/// @brief Deletes the internal pointer
	void destroy();

	/// @brief Whether this wrap_ptr holds a valid handle
	operator bool() const { return static_cast<bool>(data); }

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

/// @brief wrapper for arrays
template<typename T>
struct arr_ptr {
private:
	T* dat = nullptr;
	size_t siz = 0;
public:
	/// @brief initializes the array to a nullptr
	arr_ptr();
	arr_ptr(size_t size);
	arr_ptr(T* ptr, size_t size);

	arr_ptr(const arr_ptr& ref);
	arr_ptr(arr_ptr&& rref);
	~arr_ptr();

	arr_ptr& operator=(const arr_ptr& ref);
	arr_ptr& operator=(arr_ptr&& ref);

	/// @brief normal array access operator, DOES NOT PERFORM RANGE CHECKING 
	T& operator[](size_t ind);

	/// @brief normal array access operator, DOES NOT PERFORM RANGE CHECKING
	const T& operator[](size_t ind) const;

	/// @brief Returns the size of the arrays
	size_t size() const;

	/// @brief Whether the class holds a valid ptr
	operator bool() const;

	T* data();
	const T* data() const;

private:
	void _destroy();
	void _cpy(const arr_ptr& ref);

};

template<typename T>
wrap_ptr<T>::wrap_ptr() : data{ nullptr } { }
template<typename T>
wrap_ptr<T>::wrap_ptr(pass_ptr<T>&& ref) {
	this->data = std::move(ref);
}
template<typename T>
wrap_ptr<T>::wrap_ptr(wrap_ptr<T>&& ref) noexcept {
	this->data = std::move(ref.data);
}
template<typename T>
wrap_ptr<T>::wrap_ptr(T* vkInst) {
	this->data = std::unique_ptr<T>(vkInst);
}
template<typename T>
wrap_ptr<T>::wrap_ptr(const wrap_ptr<T>& ref) {
	_cpy(ref);
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(const wrap_ptr<T>& ref) {
	_cpy(ref);
	return *this;
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(wrap_ptr<T>&& ref) noexcept {
	this->data = std::move(ref.data);
	return *this;
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(pass_ptr<T>&& ref) noexcept {
	this->data = std::move(ref);
	return *this;
}
template<typename T>
wrap_ptr<T>& wrap_ptr<T>::operator=(T*& vkInst) {
	this->data = std::unique_ptr<T>(vkInst);
	return *this;
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
template<typename T>
void wrap_ptr<T>::destroy() {	
	this->data = nullptr; //thanks std!!
}

template<typename T>
arr_ptr<T>::arr_ptr() {}
template<typename T>
arr_ptr<T>::arr_ptr(size_t size) {
	this->dat = new T[size];
	this->siz = size;
}
template<typename T>
arr_ptr<T>::arr_ptr(T* ptr, size_t size) {
	this->dat = ptr;
	this->siz = size;
}
template<typename T>
arr_ptr<T>::arr_ptr(const arr_ptr& ref) {
	_cpy(ref);
}
template<typename T>
arr_ptr<T>::arr_ptr(arr_ptr&& rref) {
	this->dat = rref.dat;
	this->siz = rref.siz;
	rref.dat = nullptr;
	rref.siz = 0;
}
template<typename T>
arr_ptr<T>::~arr_ptr() {
	_destroy();
}

template<typename T>
arr_ptr<T>& arr_ptr<T>::operator=(const arr_ptr& ref) {
	_destroy();
	_cpy(ref);
	
	return *this;
}

template<typename T>
arr_ptr<T>& arr_ptr<T>::operator=(arr_ptr&& rref) {
	_destroy();
	this->dat = rref.dat;
	this->siz = rref.siz;
	rref.dat = nullptr;
	rref.siz = 0;
	return *this;
}

template<typename T>
T& arr_ptr<T>::operator[](size_t ind) {
#ifdef _DEBUG
	if (ind >= this->siz) {
		throw std::exception("ArrayOutofBounds!");
	}
#endif
	return this->dat[ind];
}
template<typename T>
const T& arr_ptr<T>::operator[](size_t ind) const {
#ifdef _DEBUG
	if (ind >= this->siz) {
		throw std::exception("ArrayOutofBounds!");
	}
#endif
	returnt this->dat[ind];
}
template<typename T>
size_t arr_ptr<T>::size() const {
	return this->siz;
}
template<typename T>
arr_ptr<T>::operator bool() const {
	return this->dat;
}
template<typename T>
void arr_ptr<T>::_destroy() {
	if (this->dat) {
		delete[] this->dat;
		this->dat = nullptr;
		this->siz = 0;
	}
}

template<typename T>
void arr_ptr<T>::_cpy(const arr_ptr<T>& ref) {
	this->dat = new T[ref.siz];

	if constexpr (std::is_pod<T>::value){
		std::memcpy(this->dat, ref.dat, sizeof(T) * ref.siz);
	}
	else {
		for (size_t a = 0; a < this->siz; a++) {
			this->dat[a] = ref.dat[a];
		}
	}
}
template<typename T>
T* arr_ptr<T>::data() {
	return this->dat;
}
template<typename T>
const T* arr_ptr<T>::data() const {
	return this->dat;
}