// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Reference.h"
class ResourceLoader;
class Resource;

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
	virtual ~ResourceHolder();

	void AddCallback(Callback callback);
	auto GetId() const -> const std::string& { return _id; }
	void AddLoadingDependency();
	void RemoveLoadingDependency();
	void LoadAsync(std::function<Resource*()> createFunc);
	auto Get() const -> const Resource* { return _resource; }

protected:
	friend class ResourceLoader;
	friend class ResourceManager;
	void OnAllRefsRemoved() override;

	std::string _id;
	RefPtr<ResourceHolder> _loadingParent;
	std::atomic<uint32> _nbLoadingDependencies = 0;
	std::mutex _callbackMutex;
	std::atomic<ResourceState> _state = ResourceState::UNLOADED;
	std::vector<Callback> _callbacks;
	std::atomic<Resource*> _resource = nullptr;
	std::atomic<Resource*> _loadingResource = nullptr;
	bool _loadingResult = false;
};
