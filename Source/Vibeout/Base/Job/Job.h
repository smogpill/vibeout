// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Job
{
public:
    Job() = default;

    template<typename Callable>
    Job(Callable&& callable)
        : _function([callable = std::forward<Callable>(callable)]() { callable(); })
    {
    }

    void operator()() { _function(); }

private:
    std::function<void()> _function;
};
