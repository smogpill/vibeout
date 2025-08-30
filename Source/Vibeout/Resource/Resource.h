// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Manager/ResourceManager.h"

template <class T>
class ResourceHandle
{
public:
	ResourceHandle() = default;
	explicit ResourceHandle(const std::string& id);
	ResourceHandle(const ResourceHandle& other);
	~ResourceHandle() { Release(); }
	
	auto Get() -> T* { if (_holder) return _holder->Get(); else return nullptr; }
	auto Get() const -> const T* { if (_holder) return _holder->Get(); else return nullptr; }
	void WaitReady() { if (_holder) _holder->WaitReady(); }
	void Release();
	
	auto operator=(const std::string& id) -> ResourceHandle& { *this = ResourceHandle(id); }
	auto operator=(const ResourceHandle& other) -> ResourceHandle&;
	operator bool() const { return _holder != nullptr; }

private:
	friend class ResourceManager;

	TypedResourceHolder<T>* _holder = nullptr;
};

template <class T>
ResourceHandle<T>::ResourceHandle(const std::string& id)
{
	VO_ASSERT(ResourceManager::s_instance);
	_holder = ResourceManager::s_instance->GetOrCreateHolder<T>(id);
}

template <class T>
void ResourceHandle<T>::Release()
{
	if (_holder)
	{
		_holder->RemoveRef();
		_holder = nullptr;
	}
}

template <class T>
ResourceHandle<T>::ResourceHandle(const ResourceHandle& other)
	: _holder(other._holder)
{
	if (_holder)
		_holder->AddRef();
}

template <class T>
ResourceHandle<T>& ResourceHandle<T>::operator=(const ResourceHandle& other)
{
	Release();
	if (other._holder)
	{
		_holder = other._holder;
		_holder->AddRef();
	}
	return *this;
}
