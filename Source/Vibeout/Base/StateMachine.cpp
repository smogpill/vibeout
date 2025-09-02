// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Vibeout/Base/StateMachine.h"

State::State(StateID id)
	: _id(id)
{
}

State::~State()
{
	if (_currentState)
		_currentState->OnExit(nullptr);
}

void State::AddState(State& state)
{
	VO_ASSERT(GetState(state.GetId()) == nullptr);
	_states.push_back(RefPtr(&state));
	state._parent = this;
}

void State::Update()
{
	OnUpdate();
	if (_currentState)
		_currentState->Update();
}

void State::HandleEvent(const StateEvent& event)
{
	OnEvent(event);
	if (_currentState)
		_currentState->OnEvent(event);
}

void State::SetCurrentState(StateID id)
{
	if (_currentState && _currentState->GetId() == id)
		return;

	State* previousState = _currentState;
	State* nextState = nullptr;
	for (RefPtr<State>& state : _states)
	{
		if (state->_id == id)
		{
			nextState = state.Get();
			break;
		}
	}

	if (_currentState)
		_currentState->OnExit(nextState);
	_currentState = nextState;
	if (_currentState)
		_currentState->OnEnter(previousState);
}

auto State::GetState(StateID id) const -> State*
{
	auto it = std::ranges::find_if(_states, [id](const RefPtr<State>& state) { return state->_id == id; });
	return it != _states.end() ? it->Get() : nullptr;
}

void LambdaState::OnUpdate()
{
	Base::OnUpdate();
	if (_onUpdate)
		_onUpdate();
}

void LambdaState::OnEnter(State* from)
{
	Base::OnEnter(from);
	if (_onEnter)
		_onEnter(from);
}

void LambdaState::OnExit(State* to)
{
	if (_onExit)
		_onExit(to);
	Base::OnExit(to);
}

void LambdaState::OnEvent(const StateEvent& event)
{
	Base::OnEvent(event);
	if (_onEvent)
		_onEvent(event);
}
