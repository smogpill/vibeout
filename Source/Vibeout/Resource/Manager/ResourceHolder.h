// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Reference.h"

class ResourceHolder : public RefCounted<ResourceHolder>
{
	using Base = RefCounted<ResourceHolder>;
public:
	ResourceHolder(const std::string& id);
	virtual ~ResourceHolder() = default;

	[[nodiscard]]
	bool Load();
	void WaitReady();
	auto GetId() const -> const std::string& { return _id; }

protected:
	void OnAllRefsRemoved() override;
	virtual bool OnLoad() = 0;

	std::string _id;
};

template <class T>
class TypedResourceHolder : public ResourceHolder
{
	using Base = ResourceHolder;
public:
	TypedResourceHolder(const std::string& id) : Base(id) {}
	~TypedResourceHolder() { delete _resource; }

	auto Get() const -> const T* { return _resource; }

private:
	bool OnLoad() override;

	std::atomic<T*> _resource = nullptr;
};

template <class T>
bool TypedResourceHolder<T>::OnLoad()
{
	bool result;
	std::unique_ptr<T> resource(new T(_id, result));
	if (result)
	{
		delete _resource;
		_resource = resource.release();
	}
	return result;
}
