// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

const int maxLevels = 23;
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

// Counts amount of bits in 8 bit int
uint CountBits8(in uint num)
{
	return bitCount(num & 0xff);
}

// copy sign of s into value x
float copysign(float x, float s)
{
	return uintBitsToFloat((floatBitsToUint(s) & 0x80000000u) | (floatBitsToUint(x) & 0x7fffffffu));
}

// same as above, if we know x is non-negative
float copysignp(float x, float s)
{
	return uintBitsToFloat((floatBitsToUint(s) & 0x80000000u) | floatBitsToUint(x));
}

uint GetTLASNode(in uint idx)
{
	return inTLASNodes._data[idx];
}

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

void CastContent(in Ray ray, inout CastResult result)
{
	vec3 sphereCenter = vec3(0, 0, -10);
	float sphereRadius = 1.0f;
	CastSphere(ray, sphereCenter, sphereRadius, result);
	CastTerrain(ray, result);
}

void CastTLAS(in Ray ray, inout CastResult result)
{
	Ray originalRay = ray;
	vec3 scale = vec3(terrainSize, terrainSize * terrainHeightScale, terrainSize);
	ray.o /= scale;
	ray.d *= ray._maxDist / scale;

	//-------------------------------------------------------
	// INIT
	//-------------------------------------------------------
	float tMaxStack[maxLevels + 1];
	uint nodeStack[maxLevels + 1];

	const float epsilon = exp2(-maxLevels);
	const vec3 absRayDir = abs(ray.d);
	if (absRayDir.x < epsilon) ray.d.x = copysignp(epsilon, ray.d.x);
	if (absRayDir.y < epsilon) ray.d.y = copysignp(epsilon, ray.d.y);
	if (absRayDir.z < epsilon) ray.d.z = copysignp(epsilon, ray.d.z);

	const vec3 tCoef = 1.0f / -abs(ray.d);
	vec3 tBias = tCoef * (ray.o + vec3(1.0f));

	int octantMask = 7;
	if (ray.d.x > 0.f) octantMask ^= 1, tBias.x = 3.f * tCoef.x - tBias.x;
	if (ray.d.y > 0.f) octantMask ^= 2, tBias.y = 3.f * tCoef.y - tBias.y;
	if (ray.d.z > 0.f) octantMask ^= 4, tBias.z = 3.f * tCoef.z - tBias.z;

	// curTMin & curTMax
	float curTMin;
	float curTMax;
	{
		curTMin = max(max(2.f * tCoef.x - tBias.x, 2.f * tCoef.y - tBias.y), 2.f * tCoef.z - tBias.z);
		curTMax = min(min(tCoef.x - tBias.x, tCoef.y - tBias.y), tCoef.z - tBias.z);
		curTMin = max(curTMin, 0.f);
		curTMax = min(curTMax, 1.f);
	}

	// Init iterative variables
	uint curNode = 0;
	float curScale = 0.5f;
	int curScaleIndex = int(maxLevels) - 1;
	int curIterCount = 0;

	// T in [0, 1]
	// pos in [1, 2]

	// Choose first child
	uint curChildIndex = 0;
	vec3 curCorner = vec3(1.f, 1.f, 1.f);
	{
		if (curTMin < 1.5f * tCoef.x - tBias.x) curChildIndex ^= 1, curCorner.x = 1.5f;
		if (curTMin < 1.5f * tCoef.y - tBias.y) curChildIndex ^= 2, curCorner.y = 1.5f;
		if (curTMin < 1.5f * tCoef.z - tBias.z) curChildIndex ^= 4, curCorner.z = 1.5f;
	}

	vec3 tCurPos = tBias;

	//-------------------------------------------------------
	// ITERATE
	//-------------------------------------------------------
	while (curScaleIndex < maxLevels)
	{
		// Useful because sometimes it seems that it gets stuck and crashes the driver.
		// Should be investigated though because there is no reason it could not escape, 
		// probably some mishandled singularity somewhere.
		if (curIterCount++ == 1024)
			break;

		const uint nodeData = GetTLASNode(curNode);

		// Find max t value
		const vec3 tCorner = curCorner * tCoef - tBias;
		const float tCornerMax = min(min(tCorner.x, tCorner.y), tCorner.z);

		// Permute child slots based on the mirroring
		const uint childShift = curChildIndex ^ octantMask;
		const uint childMask = nodeData << childShift;

		// Process voxel?
		if (((childMask & 0x80) != 0) && curTMin <= curTMax)
		{
			//-------------------------------------------------------
			// INTERSECT
			//-------------------------------------------------------
			const float tVoxelMax = min(curTMax, tCornerMax);

			const float halfScale = curScale * 0.5f;
			const vec3 tCenter = halfScale * tCoef + tCorner;

			if (curTMin <= tVoxelMax)
			{
				//-------------------------------------------------------
				// PUSH
				//-------------------------------------------------------
				nodeStack[curScaleIndex] = curNode;
				tMaxStack[curScaleIndex] = curTMax;

				// Find child voxNode
				curNode = GetTLASNode(curNode + CountBits8(childMask & 0xff));

				if (~curNode == 0)
				{
					//result._t = curTMin;
					//result._normal = normalize(vec3(1, 0.2, 0.8));
					Ray localRay;
					localRay.o = originalRay.o + curTMin * originalRay.d * originalRay._maxDist;
					localRay.d = originalRay.d;
					localRay._maxDist = halfScale * 4.0f;
					CastContent(localRay, result);
					return;
				}
				else
				{
					// Select child voxel that the ray enters first.

					curChildIndex = 0;
					--curScaleIndex;

					curScale = halfScale;

					if (tCenter.x > curTMin) curChildIndex ^= 1, curCorner.x += curScale;
					if (tCenter.y > curTMin) curChildIndex ^= 2, curCorner.y += curScale;
					if (tCenter.z > curTMin) curChildIndex ^= 4, curCorner.z += curScale;

					curTMax = tVoxelMax;
					continue;
				}
			}
		}

		//-------------------------------------------------------
		// ADVANCE
		//-------------------------------------------------------
		// Step along the ray.
		uint stepMask = 0;
		{
			if (tCorner.x <= tCornerMax) stepMask ^= 1, curCorner.x -= curScale;
			if (tCorner.y <= tCornerMax) stepMask ^= 2, curCorner.y -= curScale;
			if (tCorner.z <= tCornerMax) stepMask ^= 4, curCorner.z -= curScale;
		}

		// Update active t-span and flip bits of the child slot index.
		curTMin = tCornerMax;
		curChildIndex ^= stepMask;

		// Proceed with pop if the bit flips disagree with the ray direction.
		if ((curChildIndex & stepMask) != 0)
		{
			//-------------------------------------------------------
			// POP
			//-------------------------------------------------------

			// Find the highest differing bit between the new pos and old pos.
			uint differingBits = 0;
			if ((stepMask & 1) != 0) differingBits |= floatBitsToInt(curCorner.x) ^ floatBitsToInt(curCorner.x + curScale);
			if ((stepMask & 2) != 0) differingBits |= floatBitsToInt(curCorner.y) ^ floatBitsToInt(curCorner.y + curScale);
			if ((stepMask & 4) != 0) differingBits |= floatBitsToInt(curCorner.z) ^ floatBitsToInt(curCorner.z + curScale);

			curScaleIndex = (floatBitsToInt(float(differingBits)) >> 23) - 127; // position of the highest bit
			curScale = intBitsToFloat((curScaleIndex - int(maxLevels) + 127) << 23); // exp2f(scale - s_max)

			// Hack
			if (curScaleIndex < 0 || curScaleIndex >= maxLevels)
			{
				curTMin = -4.f;
				break;
			}

			// Restore the parent node from the stack. 
			curNode = nodeStack[curScaleIndex];
			curTMax = tMaxStack[curScaleIndex];

			// Round cube position and extract child slot index.
			const int shx = floatBitsToInt(curCorner.x) >> curScaleIndex;
			const int shy = floatBitsToInt(curCorner.y) >> curScaleIndex;
			const int shz = floatBitsToInt(curCorner.z) >> curScaleIndex;
			curCorner.x = intBitsToFloat(shx << curScaleIndex);
			curCorner.y = intBitsToFloat(shy << curScaleIndex);
			curCorner.z = intBitsToFloat(shz << curScaleIndex);
			curChildIndex = ((shz & 1) << 2) | ((shy & 1) << 1) | (shx & 1);
		}
	}

	// Indicate miss if we are outside the octree.
	//if (curScaleIndex >= maxLevels)
	//{
	//	curTMin = -4.f;
	//}

	// Undo mirroring of the coordinate system.
	/*if ((octantMask & 1) == 0) curCorner.x = 1.f - curCorner.x;
	if ((octantMask & 2) == 0) curCorner.y = 1.f - curCorner.y;
	if ((octantMask & 4) == 0) curCorner.z = 1.f - curCorner.z;*/
	// 	if ((octantMask & 1) != 0) curCorner.x = 3.0f - curScale - curCorner.x;
	// 	if ((octantMask & 2) != 0) curCorner.y = 3.0f - curScale - curCorner.y;
	// 	if ((octantMask & 4) != 0) curCorner.z = 3.0f - curScale - curCorner.z;
}

void CastGlobal(in Ray ray, inout CastResult result)
{
	result._t = ray._maxDist;
	CastTLAS(ray, result);
}

bool TestObstruction(vec3 pos, vec3 dir)
{
	Ray r;
	r.o = pos + dir * 0.01f;
	r.d = dir;
	r._maxDist = maxTraceDist;

	CastResult result;
	CastGlobal(r, result);
	return result._t < r._maxDist;
}