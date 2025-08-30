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

	virtual void OnLoad() {}
	void WaitReady();
	auto GetId() const -> const std::string& { return _id; }

protected:
	void OnAllRefsRemoved() override;

private:
	std::string _id;
};

template <class T>
class TypedResourceHolder : public ResourceHolder
{
public:
	~TypedResourceHolder() { delete _resource; }

	T* Get() { return _resource; }
private:
	T* _resource = nullptr;
};
