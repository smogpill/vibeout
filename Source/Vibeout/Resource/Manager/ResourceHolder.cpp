// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ResourceHolder.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"
#include "Vibeout/Base/Job/JobSystem.h"

ResourceHolder::ResourceHolder(const std::string& id)
	: _id(id)
{
}

void ResourceHolder::AddLoadingDependency()
{
	++_nbLoadingDependencies;
}

void ResourceHolder::RemoveLoadingDependency()
{
    if (--_nbLoadingDependencies == 0)
    {
        //JobSystem::s_instance->ResumeCoroutine(_loadingParent._coroutineHandle);
    }
}

auto ResourceHolder::TakeCallbacks() -> std::vector<Callback>
{
    std::scoped_lock lock(_callbackMutex);
    return std::move(_callbacks);
}

void ResourceHolder::OnAllRefsRemoved()
{
	VO_ASSERT(ResourceManager::s_instance);
	ResourceManager::s_instance->DestroyHolder(*this);
	Base::OnAllRefsRemoved();
}

void ResourceHolder::CompleteLoading(bool result)
{
    if (result)
    {
        _state = ResourceState::LOADED;
    }
    else
    {
        _state = ResourceState::FAILED;
    }

    _callbackMutex.lock();
    std::vector<std::function<void(bool)>> callbacks = std::move(_callbacks);
    _callbackMutex.unlock();
    for (auto& cb : callbacks)
    {
        JobSystem::s_instance->Enqueue([callback = std::move(cb), result]() { callback(result); });
    }

    if (_loadingParent)
    {
        _loadingParent->RemoveLoadingDependency();
        _loadingParent = nullptr;
    }
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

void ResourceHolder::LoadAsync()
{
    if (_state == ResourceState::UNLOADED)
    {
        AddRef();
        JobSystem::s_instance->Enqueue([this]()
            {
                ResourceLoader loader(*this);
                AddLoadingDependency();
                Load(loader);
                RemoveLoadingDependency();
                RemoveRef();
            });
    }
}
