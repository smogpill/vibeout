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
	ResourceHandle(const ResourceHandle& other);
	
	template <class F>
	void LoadAsync(const std::string& id, F&& onDone);
	//void ReloadAsync();
	auto Get() -> T* { if (_holder) return _holder->Get(); else return nullptr; }
	auto Get() const -> const T* { if (_holder) return _holder->Get(); else return nullptr; }
	void WaitReady() { if (_holder) _holder->WaitReady(); }
	void Release();

	auto operator=(const ResourceHandle& other) -> ResourceHandle&;
	operator bool() const { return _holder != nullptr; }

private:
	friend class ResourceManager;
	using Holder = TypedResourceHolder<T>;

	RefPtr<Holder> _holder;
};

template <class T>
template <class F>
void ResourceHandle<T>::LoadAsync(const std::string& id, F&& onDone)
{
	VO_ASSERT(ResourceManager::s_instance);
	_holder = ResourceManager::s_instance->GetOrCreateHolder<T>(id);
	auto func = [=]()
		{
			const bool result = _holder->Load();
			onDone(result);
		};
	auto future = std::async(std::launch::async, func);
}

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
