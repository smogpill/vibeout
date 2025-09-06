// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "GameRun.h"
#include "Vibeout/Game/Game.h"
#include "Vibeout/World/Node/Node.h"
#include "Vibeout/World/Camera/Camera.h"

void GameRun::OnEnter(StateMachine* from, const StateMessage& message)
{
	Base::OnEnter(from, message);
}

void GameRun::OnUpdate()
{
	Base::OnUpdate();
    UpdateCamera();
}

void GameRun::OnExit(StateMachine* to, const StateMessage& message)
{
	Base::OnExit(to, message);
}

void GameRun::UpdateCamera()
{
    Game* game = Game::s_instance;
	Camera& camera = Game::s_instance->GetCamera();
    Node& node = camera.GetNode();
    const Transform& global = node.GetGlobalTransform();
    const bool* keys = SDL_GetKeyboardState(nullptr);
    glm::vec3 localMoveIntent(0.0f);
    if (keys[SDL_SCANCODE_W]) localMoveIntent[2] -= 1.0f;
    if (keys[SDL_SCANCODE_S]) localMoveIntent[2] += 1.0f;
    if (keys[SDL_SCANCODE_A]) localMoveIntent[0] -= 1.0f;
    if (keys[SDL_SCANCODE_D]) localMoveIntent[0] += 1.0f;
    const glm::mat3 globalRotMat = glm::mat4(global.To_dmat4());
    const glm::vec3 globalMoveIntent = globalRotMat * localMoveIntent;
    const bool turbo = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    const float speed = turbo ? 10.0f : 2.0f;

    glm::dvec3 pos = global.Translation();
    pos += globalMoveIntent * speed * game->GetDeltaTime();
    camera.SetTranslation(pos);
}
