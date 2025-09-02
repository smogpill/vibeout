// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Reference.h"

using StateEventID = uint32;
using StateID = uint32;

class StateEvent
{
public:
	StateEvent(StateEventID id) : _id(id) {}
	virtual ~StateEvent() = default;

	auto GetId() const { return _id; }

private:
	StateEventID _id = 0;
};

class State : public RefCounted<State>
{
public:
	explicit State(StateID id);
	virtual ~State();

	void AddState(State& state);
	void SetCurrentState(StateID id);
	auto GetState(StateID id) const -> State*;
	auto GetId() const { return _id; }
	void Update();
	void HandleEvent(const StateEvent& event);

protected:
	virtual void OnUpdate() {}
	virtual void OnEnter(State* from) {}
	virtual void OnExit(State* to) {}
	virtual void OnEvent(const StateEvent& event) {}

private:
	State* _parent = nullptr;
	State* _currentState = nullptr;
	StateID _id = 0;
	std::vector<RefPtr<State>> _states;
};

class LambdaState : public State
{
	using Base = State;
public:
	using OnUpdateFunc = std::function<void()>;
	using OnEnterFunc = std::function<void(State*)>;
	using OnExitFunc = std::function<void(State*)>;
	using OnEventFunc = std::function<void(const StateEvent&)>;

	explicit LambdaState(StateID id) : Base(id) {}
	void SetOnUpdate(OnUpdateFunc func) { _onUpdate = func; }
	void SetOnEnter(OnEnterFunc func) { _onEnter = func; }
	void SetOnExit(OnExitFunc func) { _onExit = func; }
	void SetOnEvent(OnEventFunc func) { _onEvent = func; }

protected:
	void OnUpdate() override;
	void OnEnter(State* from) override;
	void OnExit(State* to) override;
	void OnEvent(const StateEvent& event) override;

private:
	OnUpdateFunc _onUpdate;
	OnEnterFunc _onEnter;
	OnExitFunc _onExit;
	OnEventFunc _onEvent;
};
