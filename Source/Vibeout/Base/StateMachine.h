// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Reference.h"

using StateMessageID = uint32;
using StateEventID = uint32;
using StateID = uint32;

class StateMessage
{
public:
	StateMessage() = default;
	StateMessage(StateMessageID id) : _id(id) {}
	template <class T>
	StateMessage(const T& typedId) : StateMessage(static_cast<StateMessageID>(typedId)) {}
	virtual ~StateMessage() = default;

	auto GetId() const { return _id; }

private:
	StateMessageID _id = 0;
};

class StateEvent
{
public:
	StateEvent(StateEventID id) : _id(id) {}
	template <class T>
	StateEvent(const T& typedId) : StateEvent(static_cast<StateEventID>(typedId)) {}
	virtual ~StateEvent() = default;

	auto GetId() const { return _id; }

private:
	StateEventID _id = 0;
};

class State : public RefCounted<State>
{
public:
	explicit State(StateID id);
	template <class T>
	explicit State(const T& typedId) : State(static_cast<StateID>(typedId)) {}
	virtual ~State();

	void AddState(State& state);
	void SetCurrentState(StateID id, const StateMessage& message = StateMessage());
	template <class T>
	void SetCurrentState(const T& typedId, const StateMessage& message = StateMessage()) { SetCurrentState(static_cast<StateID>(typedId), message); }
	template <class T>
	auto GetState(const T& typedId) const -> State* { return GetState(static_cast<StateID>(typedId)); }
	auto GetState(StateID id) const -> State*;
	auto GetId() const { return _id; }
	template <class T>
	auto GetTypedId() const { return static_cast<T>(_id); }
	void Update();
	void HandleEvent(const StateEvent& event);
	auto GetParent() -> State* { return _parent; }

protected:
	virtual void OnEnter(State* from, const StateMessage& message) {}
	virtual void OnExit(State* to, const StateMessage& message) {}
	virtual void OnUpdate() {}
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
	using OnEnterFunc = std::function<void(State*, const StateMessage&)>;
	using OnExitFunc = std::function<void(State*, const StateMessage&)>;
	using OnEventFunc = std::function<void(const StateEvent&)>;

	explicit LambdaState(StateID id) : Base(id) {}
	void SetOnUpdate(OnUpdateFunc func) { _onUpdate = func; }
	void SetOnEnter(OnEnterFunc func) { _onEnter = func; }
	void SetOnExit(OnExitFunc func) { _onExit = func; }
	void SetOnEvent(OnEventFunc func) { _onEvent = func; }

protected:
	void OnEnter(State* from, const StateMessage& message) override;
	void OnExit(State* to, const StateMessage& message) override;
	void OnUpdate() override;
	void OnEvent(const StateEvent& event) override;

private:
	OnUpdateFunc _onUpdate;
	OnEnterFunc _onEnter;
	OnExitFunc _onExit;
	OnEventFunc _onEvent;
};
