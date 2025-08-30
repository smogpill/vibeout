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
		VO_ASSERT(s_instance == nullptr);
		s_instance = static_cast<T*>(this);
	}
	~Singleton()
	{
		s_instance = nullptr;
	}

	static inline T* s_instance = nullptr;
};
