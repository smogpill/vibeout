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
	~RefPtr() { if (_ptr) _ptr->RemoveRef(); }

private:
	T* _ptr = nullptr;
};

template <class T>
class ConstRefPtr
{
public:
	~ConstRefPtr() { if (_ptr) _ptr->RemoveRef(); }

private:
	const T* _ptr = nullptr;
};

template <class T>
void RefCounted<T>::RemoveRef() const
{
	if (--_nbRefs == 0)
		OnAllRefsRemoved();
}
