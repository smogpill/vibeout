// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Manager/ResourceHolder.h"

class Resource
{
public:
	virtual ~Resource() {}
};

template <class T>
class ResourceHandle
{
public:
	ResourceHandle() = default;
	ResourceHandle(const ResourceHandle& other);
	
	//void ReloadAsync();
	auto Get() -> T* { if (_holder) return _holder->Get(); else return nullptr; }
	auto Get() const -> const T* { if (_holder) return _holder->Get(); else return nullptr; }
	void Release();
	void AddCallback(std::function<void(bool)> callback) { _holder->AddCallback(callback); }
	void LoadAsync();

	auto operator=(const ResourceHandle& other) -> ResourceHandle&;
	operator bool() const { return _holder != nullptr; }

private:
	friend class ResourceManager;
	friend class ResourceLoader;
	using Holder = TypedResourceHolder<T>;

	ResourceHandle(Holder* holder) : _holder(holder) {}

	RefPtr<Holder> _holder;
};

template <class T>
void ResourceHandle<T>::Release()
{
	_holder.Release();
}

template <class T>
ResourceHandle<T>::ResourceHandle(const ResourceHandle& other)
	: _holder(other._holder)
{
}

template <class T>
ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle& other)
{
	_holder = other._holder;
	return *this;
}

template <class T>
void ResourceHandle<T>::LoadAsync()
{
	if (_holder)
	{
		_holder->LoadAsync();
	}
}
