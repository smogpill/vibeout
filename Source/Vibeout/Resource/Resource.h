// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Manager/ResourceHolder.h"

class Resource
{
public:
	virtual ~Resource() {}

	virtual bool OnLoad(ResourceLoader& loader) = 0 { return true; }
};

template <class T>
class ResourceHandle
{
public:
	ResourceHandle() = default;
	ResourceHandle(const ResourceHandle& other) : _holder(other._holder) {}
	
	//void ReloadAsync();
	auto Get() -> const T* { if (_holder) return static_cast<const T*>(_holder->Get()); else return nullptr; }
	//auto Get() const -> const T* { if (_holder) return static_cast<const T*>(_holder->Get()); else return nullptr; }
	void Release() { _holder.Release(); }
	void AddCallback(std::function<void(bool)> callback) { VO_ASSERT(_holder); _holder->AddCallback(callback); }
	void LoadAsync();

	auto operator=(const ResourceHandle& other) -> ResourceHandle& { _holder = other._holder; return *this; }
	operator bool() const { return _holder != nullptr; }

private:
	friend class ResourceManager;
	friend class ResourceLoader;

	ResourceHandle(ResourceHolder* holder) : _holder(holder) {}

	RefPtr<ResourceHolder> _holder;
};

template <class T>
void ResourceHandle<T>::LoadAsync()
{
	if (_holder)
		_holder->LoadAsync([]() { return new T(); });
}
