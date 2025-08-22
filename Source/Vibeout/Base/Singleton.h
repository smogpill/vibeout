// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

template <class T>
class Singleton
{
public:
	Singleton()
	{
		VO_ASSERT(_instance == nullptr);
		_instance = static_cast<T*>(this);
	}
	~Singleton()
	{
		_instance = nullptr;
	}

	static inline T* _instance = nullptr;
};
