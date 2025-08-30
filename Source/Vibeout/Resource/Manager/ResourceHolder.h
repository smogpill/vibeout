// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Reference.h"
class ResourceLoader;

class ResourceHolder : public RefCounted<ResourceHolder>
{
	using Base = RefCounted<ResourceHolder>;
public:
	ResourceHolder(const std::string& id);
	virtual ~ResourceHolder() = default;

	[[nodiscard]]
	virtual bool Load(ResourceLoader& loader) = 0;
	void WaitReady();
	auto GetId() const -> const std::string& { return _id; }
	void AddLoadingDependency();
	void RemoveLoadingDependency();

protected:
	friend class ResourceLoader;
	void OnAllRefsRemoved() override;

	std::string _id;
	RefPtr<ResourceHolder> _loadingParent;
	std::atomic<uint32> _nbLoadingDependencies = 0;
};

template <class T>
class TypedResourceHolder : public ResourceHolder
{
	using Base = ResourceHolder;
public:
	TypedResourceHolder(const std::string& id) : Base(id) {}
	~TypedResourceHolder() { delete _resource; }

	bool Load(ResourceLoader& loader) override;
	auto Get() const -> const T* { return _resource; }

private:
	std::atomic<T*> _resource = nullptr;
};

template <class T>
bool TypedResourceHolder<T>::Load(ResourceLoader& loader)
{
	std::unique_ptr<T> resource(new T());
	if (resource->OnLoad(loader))
	{
		delete _resource;
		_resource = resource.release();
		return true;
	}
	return false;
}
