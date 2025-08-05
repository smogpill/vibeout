// Copyright(c) 2022 Jounayd Id Salah

// Better temporal noise accumulation when using the golden ratio
// Golden ratio = (1.0f + sqrtf(5.0f)) / 2.0f = 1.61803398875f
// https://blog.demofox.org/2017/10/31/animating-noise-for-integration-over-time/
const float goldenRatio = 1.61803398875f;

#define ADD_TEMPORAL_NOISE_FUNC(_type_) \
void AddTemporalNoise(inout _type_ noise, uint frame) \
{ \
	noise += _type_(frame * goldenRatio); \
	noise -= floor(noise); \
}

ADD_TEMPORAL_NOISE_FUNC(vec2)
ADD_TEMPORAL_NOISE_FUNC(vec3)

uint wang_hash(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}

float uintToFloat(uint x)
{
	return uintBitsToFloat(0x3f800000 | (x >> 9)) - 1.f;
}

uint xorshift(inout uint rngState)
{
	rngState ^= rngState << 13;
	rngState ^= rngState >> 17;
	rngState ^= rngState << 5;
	return rngState;
}

float rand(inout uint rngState)
{
	return uintToFloat(xorshift(rngState));
}

uvec4 pcg4d(uvec4 v)
{
	v = v * 1664525u + 1013904223u;
	v.x += v.y * v.w;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.w += v.y * v.z;
	v = v ^ (v >> 16u);
	v.x += v.y * v.w;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.w += v.y * v.z;
	return v;
}

float RandomFloat01(inout uint state)
{
	return float(wang_hash(state)) / 4294967296.0;
	//return rand(state);
}

uint rand_pcg(inout uint rngState)
{
	uint state = rngState;
	rngState = rngState * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

vec3 RandomUnitVector(inout uint state)
{
	float z = RandomFloat01(state) * 2.0f - 1.0f;
	float a = RandomFloat01(state) * PI * 2.0f;
	float r = sqrt(1.0f - z * z);
	vec2 xy = vec2(cos(a), sin(a)) * r;
	return vec3(xy, z);
}

vec3 RandomHemisphere(inout uint state, vec3 n)
{
	vec3 r = RandomUnitVector(state);
	return dot(r, n) < 0.0 ? -r : r;
}

// Random cosine-weighted unit vector on a hemisphere
// Unit vector + random unit vector
vec3 RandomCosineHemisphere(inout uint state, vec3 n)
{
	return normalize(RandomUnitVector(state) + n);
}

// Animated Interleaved Gradient Noise
// [Next generation post processing in Call of Duty. Jorge Jimenez. 2014](http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare)
// https://github.com/gamehacker1999/DX12Engine/blob/9ce4fac6241574f76a1af992972a66df1a519155/PixelShaderPBR.hlsl
// [Interleaved Gradient Noise: A Different Kind of Low Discrepancy Sequence](https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/)
float IGN(uint pixelX, uint pixelY, uint frame)
{
	frame = frame % 64; // need to periodically reset frame to avoid numerical issues
	float x = float(pixelX) + 5.588238f * float(frame);
	float y = float(pixelY) + 5.588238f * float(frame);
	return mod(52.9829189f * mod(0.06711056f * float(x) + 0.00583715f * float(y), 1.0f), 1.0f);
}

// From http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float rand(vec2 co)
{
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt = dot(co.xy, vec2(a, b));
	float sn = mod(dt, 3.14);
	return fract(sin(sn) * c);
}

uint ComputeRngStateFromFrame(uvec2 coords, uint frameIdx)
{
	return uint(coords.x * uint(1973) + coords.y * uint(9277) + uint(frameIdx) * uint(26699)) | uint(1);
}
