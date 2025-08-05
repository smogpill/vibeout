// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

float DFBox(vec2 p, vec2 b)
{
    vec2 d = abs(p - b * 0.5) - b * 0.5;
    return min(max(d.x, d.y), 0.) + length(max(d, 0.));
}
