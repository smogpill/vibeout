// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Reference.h"
class ResourceLoader;

enum class ResourceState
{
	UNLOADED,
	LOADING,
	LOADED,
	FAILED
};

template <class T>
struct LoadResult
{
	std::unique_ptr<T> _resource = nullptr;
	bool _result = false;
};

class ResourceHolder : public RefCounted<ResourceHolder>
{
	using Base = RefCounted<ResourceHolder>;
public:
	using Callback = std::function<void(bool)>;
	ResourceHolder(const std::string& id);
	virtual ~ResourceHolder() = default;

	void AddCallback(Callback callback);
	auto GetId() const -> const std::string& { return _id; }
	void AddLoadingDependency();
	void RemoveLoadingDependency();
	auto TakeCallbacks() -> std::vector<Callback>;
	void LoadAsync();

protected:
	friend class ResourceLoader;
	friend class ResourceManager;
	void OnAllRefsRemoved() override;
	void CompleteLoading(bool result);
	virtual void Load(ResourceLoader& loader) = 0;

	std::string _id;
	RefPtr<ResourceHolder> _loadingParent;
	std::atomic<uint32> _nbLoadingDependencies = 0;
	std::mutex _callbackMutex;
	std::atomic<ResourceState> _state = ResourceState::UNLOADED;
	std::vector<Callback> _callbacks;
};

template <class T>
class TypedResourceHolder final : public ResourceHolder
{
	using Base = ResourceHolder;
public:
	TypedResourceHolder(const std::string& id) : Base(id) {}
	~TypedResourceHolder() { delete _resource; }

	auto Get() const -> const T* { return _resource; }

private:
	void Load(ResourceLoader& loader) override;

	std::atomic<T*> _resource = nullptr;
};

template <class T>
void TypedResourceHolder<T>::Load(ResourceLoader& loader)
{
	std::unique_ptr<T> resource(new T());
	const bool result = resource->OnLoad(loader);
	if (result)
	{
		delete _resource;
		_resource = resource.release();
	}
	CompleteLoading(result);
}
