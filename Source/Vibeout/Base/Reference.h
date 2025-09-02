// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

template <class T>
class RefCounted
{
public:
	virtual ~RefCounted() = default;

	void AddRef() const { ++_nbRefs; }
	void RemoveRef() const;
	void DisableRefCounting() const { _nbRefs = s_disabledRefCounting; }

protected:
	virtual void OnAllRefsRemoved() { delete this; }

private:
	static constexpr uint32 s_disabledRefCounting = 0xdddddddd;

	mutable std::atomic<uint32> _nbRefs = 0;
};

template <class T>
class RefPtr
{
public:
	RefPtr() = default;
	RefPtr(const RefPtr& other) : _ptr(other._ptr) { if (_ptr) _ptr->AddRef(); }
	RefPtr(RefPtr&& other) : _ptr(other._ptr) { other._ptr = nullptr; }
	explicit RefPtr(T* ptr) : _ptr(ptr) { if (_ptr) _ptr->AddRef(); }
	~RefPtr() { Release(); }

	auto Get() const -> T* { return _ptr; }
	void Release() { if (_ptr) _ptr->RemoveRef(); }

	auto operator=(const RefPtr& other) -> RefPtr&;
	auto operator=(RefPtr&& other) -> RefPtr&;
	auto operator=(T* ptr) -> RefPtr&;
	auto operator->() const -> T* { return _ptr; }
	auto operator*() const -> T& { VO_ASSERT(_ptr); return *_ptr; }
	bool operator==(const T* p) const { return _ptr == p; }
	bool operator!=(const T* p) const { return _ptr != p; }
	operator bool() const { return _ptr != nullptr; }

private:
	T* _ptr = nullptr;
};

template <class T>
class ConstRefPtr
{
public:
	~ConstRefPtr() { Release(); }
	void Release() { if (_ptr) _ptr->RemoveRef(); }
private:
	const T* _ptr = nullptr;
};

template <class T>
void RefCounted<T>::RemoveRef() const
{
	if (--_nbRefs == 0)
		const_cast<RefCounted<T>*>(this)->OnAllRefsRemoved();
}

template <class T>
auto RefPtr<T>::operator=(const RefPtr& other) -> RefPtr&
{
	T* oldPtr = _ptr;
	_ptr = other._ptr;
	if (_ptr)
		_ptr->AddRef();
	if (oldPtr)
		oldPtr->RemoveRef();
	return *this;
}

template <class T>
auto RefPtr<T>::operator=(RefPtr&& other) -> RefPtr&
{
	std::swap(_ptr, other._ptr);
	return *this;
}

template <class T>
auto RefPtr<T>::operator=(T* ptr) -> RefPtr&
{
	if (_ptr != ptr)
	{
		T* oldPtr = _ptr;
		_ptr = ptr;
		if (_ptr)
			_ptr->AddRef();
		if (oldPtr)
			oldPtr->RemoveRef();
	}
	return *this;
}
