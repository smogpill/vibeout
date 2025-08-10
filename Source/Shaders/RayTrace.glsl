// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

const float maxTraceDist = 1e6f;
const float terrainSize = 10.0f;
const float terrainHeightScale = 0.8f;

struct Ray
{
	vec3 o;
	vec3 d;
	float _maxDist;
};

struct CastResult
{
	float _t;
	uint _objectID;
	vec2 _uv;
	vec3 _normal;
};

vec3 ComputeTerrainNormal(vec2 uv)
{
	const vec2 texelSize = 1.0 / vec2(textureSize(TEX_HEIGHTMAP, 0));
	float hL = texture(TEX_HEIGHTMAP, uv - (texelSize.x, 0)).r;
	float hR = texture(TEX_HEIGHTMAP, uv + (texelSize.x, 0)).r;
	float hU = texture(TEX_HEIGHTMAP, uv - (0, texelSize.y)).r;
	float hD = texture(TEX_HEIGHTMAP, uv + (0, texelSize.y)).r;
	float dHdU = (hR - hL) * terrainHeightScale;
	float dHdV = (hU - hD) * terrainHeightScale;
	vec3 tangent = normalize(vec3(2.0 * texelSize.x, dHdU, 0.0));
	vec3 bitangent = normalize(vec3(0.0, dHdV, 2.0 * texelSize.y));
	return -normalize(cross(tangent, bitangent));
}

// Returns true if ray intersects AABB, with tMin and tMax as entry/exit distances
bool RayAABBIntersection(vec3 rayOrigin, vec3 rayDir, vec3 aabbMin, vec3 aabbMax, out float tMin, out float tMax)
{
	// Inverse direction avoids division and handles infinity correctly
	vec3 invDir = 1.0 / rayDir;

	// Calculate intersections with AABB planes
	vec3 t0 = (aabbMin - rayOrigin) * invDir;
	vec3 t1 = (aabbMax - rayOrigin) * invDir;

	// Ensure t0 holds near intersections, t1 holds far intersections
	vec3 tNear = min(t0, t1);
	vec3 tFar = max(t0, t1);

	// Find the farthest near and nearest far points
	tMin = max(max(tNear.x, tNear.y), tNear.z);
	tMax = min(min(tFar.x, tFar.y), tFar.z);

	// Check if the ray misses the AABB entirely
	return tMin <= tMax && tMax >= 0.0;
}

void CastTerrain(in Ray ray, inout CastResult result)
{
	float initialStepSize = 0.0001f;
	vec3 localTerrainOrigin = vec3(-0.5f, 0.0f, -0.5f);
	vec3 localOrigin = ray.o / terrainSize - localTerrainOrigin;

	// Compute intersection with the terrain AABB
	float tMax;
	{
		vec3 aabbMin = vec3(0, 0, 0);
		vec3 aabbMax = vec3(1, terrainHeightScale, 1);

		// Inverse direction avoids division and handles infinity correctly
		vec3 invDir = 1.0 / ray.d;

		// Calculate intersections with AABB planes
		vec3 tInter0 = (aabbMin - localOrigin) * invDir;
		vec3 tInter1 = (aabbMax - localOrigin) * invDir;

		vec3 tFar = max(tInter0, tInter1);

		tMax = min(min(tFar.x, tFar.y), tFar.z);
	}

	tMax = min(tMax, ray._maxDist / terrainSize);

	float t = 0.0f;

	// Coarse
	for (; t < tMax; t += initialStepSize)
	{
		vec3 pos = localOrigin + ray.d * t;
		vec2 uv = clamp(pos.xz, 0.0, 1.0);
		if (uv.x != pos.x || uv.y != pos.z)
			// break;
			continue;
		float height = textureLod(TEX_HEIGHTMAP, uv, 0.0).r * terrainHeightScale;
		if (pos.y <= height) break;
		t += initialStepSize;
	}

	if (t >= tMax)
		return;

	// Refine
	float t0 = max(0.0, t - initialStepSize);
	float t1 = t;
	for (int i = 0; i < 8; ++i)
	{
		t = mix(t0, t1, 0.5);
		vec3 pos = localOrigin + ray.d * t;
		vec2 uv = clamp(pos.xz, 0.0, 1.0);
		float height = textureLod(TEX_HEIGHTMAP, uv, 0.0).r * terrainHeightScale;
		if (pos.y < height) t1 = t;
		else t0 = t;
	}

	float globalT = t * terrainSize;
	if (globalT < result._t)
	{
		// Final position and normal
		vec3 hitPos = localOrigin + ray.d * t;
		vec2 uv = clamp(hitPos.xz, 0.0, 1.0);
		result._t = globalT;
		result._normal = ComputeTerrainNormal(uv);
		result._objectID = 0;
		result._uv = uv;
	}
}

void CastSphere(in Ray ray, in vec3 ce, float ra, inout CastResult result)
{
	vec3 oc = ray.o - ce;
	float b = dot(oc, ray.d);
	float c = dot(oc, oc) - ra * ra;
	float h = b * b - c;
	if (h < 0.0)
		return;
	h = sqrt(h);
	const float t = -b - h;
	if (t < 0.0f)
		return;
	if (t < result._t)
	{
		result._t = t;
		result._normal = normalize(ray.o + ray.d * t - ce);
		result._objectID = 1;
	}
}

void CastGlobal(in Ray ray, inout CastResult result)
{
	result._t = ray._maxDist;

	vec3 sphereCenter = vec3(0, 0, -10);
	float sphereRadius = 1.0f;

	CastSphere(ray, sphereCenter, sphereRadius, result);
	CastTerrain(ray, result);
}
