// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ResourceHolder.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"
#include "Vibeout/Resource/Resource.h"
#include "Vibeout/Base/Job/JobSystem.h"

ResourceHolder::ResourceHolder(const std::string& id)
	: _id(id)
{
}

ResourceHolder::~ResourceHolder()
{
    delete _resource;
    delete _loadingResource;
}

void ResourceHolder::AddLoadingDependency()
{
	++_nbLoadingDependencies;
}

void ResourceHolder::RemoveLoadingDependency()
{
    if (--_nbLoadingDependencies == 0)
    {
        _callbackMutex.lock();
        if (_loadingResult)
        {
            Resource* resource = _resource.exchange(_loadingResource.exchange(nullptr));
            delete resource;
            _state = ResourceState::LOADED;
        }
        else
        {
            Resource* resource = _loadingResource.exchange(nullptr);
            delete resource;
            if (_state != ResourceState::LOADED)
                _state = ResourceState::FAILED;
        }
        std::vector<std::function<void(bool)>> callbacks = std::move(_callbacks);
        _callbackMutex.unlock();
        for (auto& cb : callbacks)
        {
            JobSystem::s_instance->Enqueue([callback = std::move(cb), loadingResult = _loadingResult]() { callback(loadingResult); });
        }

        if (_loadingParent)
        {
            if (!_loadingResult)
                _loadingParent->_loadingResult = false;
            _loadingParent->RemoveLoadingDependency();
            _loadingParent = nullptr;
        }
    }
}

void ResourceHolder::OnAllRefsRemoved()
{
	VO_ASSERT(ResourceManager::s_instance);
	ResourceManager::s_instance->DestroyHolder(*this);
	Base::OnAllRefsRemoved();
}

void ResourceHolder::AddCallback(Callback callback)
{
    std::scoped_lock lock(_callbackMutex);
    switch (_state)
    {
    case ResourceState::LOADED:
    {
        JobSystem::s_instance->Enqueue([cb = std::move(callback)]() { cb(true); });
        break;
    }
    case ResourceState::FAILED:
    {
        JobSystem::s_instance->Enqueue([cb = std::move(callback)]() { cb(false); });
        break;
    }
    default:
    {
        _callbacks.push_back(std::move(callback));
        break;
    }
    }
}

void ResourceHolder::LoadAsync(std::function<Resource*()> createFunc)
{
    ResourceState expected = ResourceState::UNLOADED;
    if (_state.compare_exchange_strong(expected, ResourceState::LOADING))
    {
        AddRef();
        JobSystem::s_instance->Enqueue([this, create = std::move(createFunc)]()
            {
                Resource* oldResource = _loadingResource.exchange(create());
                delete oldResource;
                ResourceLoader loader(*this);
                AddLoadingDependency();
                _loadingResult = _loadingResource.load()->OnLoad(loader);
                RemoveLoadingDependency();
                RemoveRef();
            });
    }
}
