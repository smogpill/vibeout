// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Vibeout/Base/StateMachine.h"

StateMachine::StateMachine(StateID id)
	: _id(id)
{
}

StateMachine::~StateMachine()
{
	if (_currentState)
		_currentState->OnExit(nullptr, StateMessage());
}

void StateMachine::AddState(StateMachine& state)
{
	VO_ASSERT(GetState(state.GetId()) == nullptr);
	_states.push_back(RefPtr(&state));
	state._parent = this;
}

void StateMachine::Update()
{
	OnUpdate();
	if (_currentState)
		_currentState->Update();
}

void StateMachine::HandleEvent(const StateEvent& event)
{
	OnEvent(event);
	if (_currentState)
		_currentState->OnEvent(event);
}

void StateMachine::SetCurrentState(StateID id, const StateMessage& message)
{
	if (_currentState && _currentState->GetId() == id)
		return;

	StateMachine* previousState = _currentState;
	StateMachine* nextState = nullptr;
	for (RefPtr<StateMachine>& state : _states)
	{
		if (state->_id == id)
		{
			nextState = state.Get();
			break;
		}
	}

	if (_currentState)
		_currentState->OnExit(nextState, message);
	_currentState = nextState;
	if (_currentState)
		_currentState->OnEnter(previousState, message);
}

auto StateMachine::GetState(StateID id) const -> StateMachine*
{
	auto it = std::ranges::find_if(_states, [id](const RefPtr<StateMachine>& state) { return state->_id == id; });
	return it != _states.end() ? it->Get() : nullptr;
}

void LambdaStateMachine::OnUpdate()
{
	Base::OnUpdate();
	if (_onUpdate)
		_onUpdate();
}

void LambdaStateMachine::OnEnter(StateMachine* from, const StateMessage& message)
{
	Base::OnEnter(from, message);
	if (_onEnter)
		_onEnter(from, message);
}

void LambdaStateMachine::OnExit(StateMachine* to, const StateMessage& message)
{
	if (_onExit)
		_onExit(to, message);
	Base::OnExit(to, message);
}

void LambdaStateMachine::OnEvent(const StateEvent& event)
{
	Base::OnEvent(event);
	if (_onEvent)
		_onEvent(event);
}
