// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

float saturate(float x)
{
    return clamp(x, 0, 1);
}

vec3 saturate(vec3 x)
{
    return clamp(x, vec3(0), vec3(1));
}

const float PI = 3.14159265359;