// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

const float I3 = 1. / 3.;
const float I16 = 1. / 16.;
const float I32 = 1. / 32.;
const float I64 = 1. / 64.;
const float I128 = 1. / 128.;
const float I256 = 1. / 256.;
const float I300 = 1. / 300.;
const float I512 = 1. / 512.;
const float I1024 = 1. / 1024.;
const float I2048 = 1. / 2048.;

//Non-optimal vec2/vec3 to float functions
vec3 FloatToVec3(float v)
{
    float x = fract(v);
    float z = floor(v * I300);
    float y = floor(v - z * 300.) * I300;
    return vec3(x, y, z * I300);
}

float Vec3ToFloat(vec3 v)
{
    return min(v.x, 0.998) + min(299., floor(v.y * 300. + 0.5)) + floor(v.z * 300. + 0.5) * 300.;
}

vec2 FloatToVec2(float v)
{
    return vec2((floor(fract(v) * 2048.) + 0.5) * I2048, (floor(v) + 0.5) * I2048);
}

float Vec2ToFloat(vec2 v)
{
    return min(v.x, 0.999) + min(floor(v.y * 2048.), 2048.);
}
