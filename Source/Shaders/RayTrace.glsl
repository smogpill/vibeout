// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

const int maxLevels = 23;
const float maxTraceDist = 1e5f;
const float rootHalfSide = 32.0f;
const uint levelNodeOffsets[] = { 0, 1, 9, 73, 585, 4681, 37449, 299593, 2396745, 19173961/*, 153391689*/ };

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

struct Ray
{
	vec3 o;
	vec3 d;
};

uint GetVoxelData(in uint idx)
{
	return _voxelBlocks[nonuniformEXT(idx >> VOXEL_BLOCK_INDEX_SHIFT)]._nodes[idx & VOXEL_BLOCK_OFFSET_MASK];
	//return inVoxelData._nodes[idx];
}

uint GetTLASNode(in uint level, in uint idx)
{
	const uint offset = levelNodeOffsets[level] + idx;
	return (inTLASNodes._data[offset / 4] >> (8 * (offset % 4))) & 0xff;
}

uint GetTLASLeaf(in uint idx)
{
	return inTLASLeaves._data[idx];
}

uint GetBroadphaseCell(in uint idx)
{
	return inBroadphaseData._cells[idx];
}

//-------------------------------------------------------
vec4 CastLocal(in Ray ray, uint voxelPtr, in uint drawDepth, out vec3 norm)
{

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
	uint curNode = voxelPtr;
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

		const uint nodeData = GetVoxelData(curNode);

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

				/*
				if ((childMask & 0x80) == 0)
				{
					//tCurPos		= tCorner - curScale * tCoef;

					break; // at curTMin (overridden with tVoxelMin).
				}
				*/

				if (curScaleIndex == maxLevels - drawDepth)
				{
					// Compute normal
					{
						const vec3 tCorner2 = (curCorner + curScale) * tCoef - tBias;
						const float tCornerMin = max(max(tCorner2.x, tCorner2.y), tCorner2.z);

						if (tCorner2.x == tCornerMin)
							norm = vec3(-1, 0, 0);
						else if (tCorner2.y == tCornerMin)
							norm = vec3(0, -1, 0);
						else
							norm = vec3(0, 0, -1);

						if ((octantMask & 1) != 0) norm.x = -norm.x;
						if ((octantMask & 2) != 0) norm.y = -norm.y;
						if ((octantMask & 4) != 0) norm.z = -norm.z;
					}
					break;
				}

				// Find child voxNode
				curNode = GetVoxelData(curNode + CountBits8(childMask & 0xff));

				if (~curNode == 0)
				{
					// Compute normal
					{
						const vec3 tCorner2 = (curCorner + curScale) * tCoef - tBias;
						const float tCornerMin = max(max(tCorner2.x, tCorner2.y), tCorner2.z);

						if (tCorner2.x == tCornerMin)
							norm = vec3(-1, 0, 0);
						else if (tCorner2.y == tCornerMin)
							norm = vec3(0, -1, 0);
						else
							norm = vec3(0, 0, -1);

						if ((octantMask & 1) != 0) norm.x = -norm.x;
						if ((octantMask & 2) != 0) norm.y = -norm.y;
						if ((octantMask & 4) != 0) norm.z = -norm.z;
					}
					break;
				}

				/*
				if (GetVoxelData(curNode) == 0)
				{
					// Compute normal
					{
						const vec3 tCorner2 = (curCorner + curScale) * tCoef - tBias;
						const float tCornerMin = max(max(tCorner2.x, tCorner2.y), tCorner2.z);

						if (tCorner2.x == tCornerMin)
							norm = vec3(-1, 0, 0);
						else if (tCorner2.y == tCornerMin)
							norm = vec3(0, -1, 0);
						else
							norm = vec3(0, 0, -1);

						if ((octantMask & 1) != 0) norm.x = -norm.x;
						if ((octantMask & 2) != 0) norm.y = -norm.y;
						if ((octantMask & 4) != 0) norm.z = -norm.z;
					}

					break;
				}
				*/

				// Select child voxel that the ray enters first.

				curChildIndex = 0;
				--curScaleIndex;

				// Hack
				if (curScaleIndex < 0 || curScaleIndex >= maxLevels)
				{
					curTMin = -4.f;
					break;
				}

				curScale = halfScale;

				if (tCenter.x > curTMin) curChildIndex ^= 1, curCorner.x += curScale;
				if (tCenter.y > curTMin) curChildIndex ^= 2, curCorner.y += curScale;
				if (tCenter.z > curTMin) curChildIndex ^= 4, curCorner.z += curScale;

				curTMax = tVoxelMax;
				continue;
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
	if (curScaleIndex >= maxLevels)
	{
		curTMin = -4.f;
	}

	// Undo mirroring of the coordinate system.
	/*if ((octantMask & 1) == 0) curCorner.x = 1.f - curCorner.x;
	if ((octantMask & 2) == 0) curCorner.y = 1.f - curCorner.y;
	if ((octantMask & 4) == 0) curCorner.z = 1.f - curCorner.z;*/
	// 	if ((octantMask & 1) != 0) curCorner.x = 3.0f - curScale - curCorner.x;
	// 	if ((octantMask & 2) != 0) curCorner.y = 3.0f - curScale - curCorner.y;
	// 	if ((octantMask & 4) != 0) curCorner.z = 3.0f - curScale - curCorner.z;

	return vec4(curTMin, 0, 0, curNode);
}

#if 1
vec4 CastGlobal(in Ray ray, uint drawDepth, out vec3 norm)
{
	const uint globalMaxLevel = NB_WORLD_TLAS_LEVELS - 2;
	const uint nbLeavesPerSide = 1u << (NB_WORLD_TLAS_LEVELS - 1);
	const float chunkSizeInMeters = 64.0f;
	const float maskSizeInMeters = nbLeavesPerSide * chunkSizeInMeters;

	//-------------------------------------------------------
	// INIT
	//-------------------------------------------------------
	float tMaxStack[maxLevels + 1];
	uint nodeStack[maxLevels + 1];
	const float epsilon = exp2(-maxLevels);

	const vec3 raySegmentInMask = ray.d / maskSizeInMeters;
	const vec3 rayOriginInMask = (ray.o / chunkSizeInMeters - _globalUBO._worldMaskOrigin) / nbLeavesPerSide;
	const vec3 rayDirInMask = normalize(raySegmentInMask);

	vec3 raySeg = raySegmentInMask;
	const vec3 origin = rayOriginInMask + vec3(1.0f);

	const vec3 absRayDir = abs(raySeg);
	if (absRayDir.x < epsilon) raySeg.x = copysignp(epsilon, raySeg.x);
	if (absRayDir.y < epsilon) raySeg.y = copysignp(epsilon, raySeg.y);
	if (absRayDir.z < epsilon) raySeg.z = copysignp(epsilon, raySeg.z);

	const vec3 tCoef = 1.0f / -abs(raySeg);
	vec3 tBias = tCoef * origin;

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

	const float scale = 2.0f * rootHalfSide;
	Ray rayLocal;
	rayLocal.d = ray.d / scale;

	//-------------------------------------------------------
	// ITERATE
	//-------------------------------------------------------
	while (curScaleIndex < maxLevels)
	{
		const int level = maxLevels - 1 - curScaleIndex;

		const uint nodeData = GetTLASNode(level, curNode);

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
				const uint childNode = (curNode << 3) | (childShift ^ 7);
				if (level == globalMaxLevel)
				{
					//const prVDAG* voxelData = _voxelChunksInScope[childNode];

					if (true)
					{
						const uint voxelPtr = GetTLASLeaf(childNode);

						vec3 chunkPos = curCorner;
						if ((octantMask & 1) == 0) chunkPos.x = 3.0f - curScale - chunkPos.x;
						if ((octantMask & 2) == 0) chunkPos.y = 3.0f - curScale - chunkPos.y;
						if ((octantMask & 4) == 0) chunkPos.z = 3.0f - curScale - chunkPos.z;
						vec3 normal;
						rayLocal.o = (rayOriginInMask - (chunkPos - vec3(1, 1, 1))) * nbLeavesPerSide;
						vec4 result = CastLocal(rayLocal, voxelPtr, drawDepth, normal);
						if (result.x > 0)
						{
							norm = normal;
							return result;
						}
					}
					else
					{
						norm = vec3(1, 0, 0);
						return vec4(0.01f, 0, 0, 0);
					}
				}
				else
				{
					//-------------------------------------------------------
					// PUSH
					//-------------------------------------------------------
					nodeStack[curScaleIndex] = curNode;
					tMaxStack[curScaleIndex] = curTMax;

					// Select child voxel that the ray enters first.
					curNode = childNode;

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
	if (curScaleIndex >= maxLevels)
	{
		curTMin = -4.f;
	}

	// Undo mirroring of the coordinate system.
	/*if ((octantMask & 1) == 0) curCorner.x = 1.f - curCorner.x;
	if ((octantMask & 2) == 0) curCorner.y = 1.f - curCorner.y;
	if ((octantMask & 4) == 0) curCorner.z = 1.f - curCorner.z;*/
	// 	if ((octantMask & 1) != 0) curCorner.x = 3.0f - curScale - curCorner.x;
	// 	if ((octantMask & 2) != 0) curCorner.y = 3.0f - curScale - curCorner.y;
	// 	if ((octantMask & 4) != 0) curCorner.z = 3.0f - curScale - curCorner.z;

	return vec4(curTMin, 0, 0, curNode);
}
#endif

//-------------------------------------------------------
#if 0
vec4 CastGlobal(in Ray ray, uint drawDepth, out vec3 norm)
{
	const vec3 absRayDir = abs(ray.d);

	const float epsilon = 1e-20f;

	if (absRayDir.x < epsilon) ray.d.x = copysignp(epsilon, ray.d.x);
	if (absRayDir.y < epsilon) ray.d.y = copysignp(epsilon, ray.d.y);
	if (absRayDir.z < epsilon) ray.d.z = copysignp(epsilon, ray.d.z);

	const vec3 dir = normalize(ray.d);
	const vec3 invDir = 1.0f / dir;

	const uint gridWidth = 256;
	const uint halfGridWidth = gridWidth / 2;
	const ivec3 gridOrigin = ivec3(-halfGridWidth);
	const float scale = 2.0f * rootHalfSide;

	vec3 va = ray.o / scale - gridOrigin;
	vec3 vb = (ray.o + ray.d) / scale - gridOrigin;
	ivec3 a = ivec3(va);
	ivec3 b = ivec3(vb);

	// http://www.cse.yorku.ca/%7Eamana/research/grid.pdf
	// https://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_4_Spatial_Subdivisions.shtml
	// https://stackoverflow.com/questions/5186939/algorithm-for-drawing-a-4-connected-line

	//ivec3 d = abs(b - a);

	ivec3 posLocal = a;

	ivec3 step;
	step.x = dir.x > 0 ? 1 : -1;
	step.y = dir.y > 0 ? 1 : -1;
	step.z = dir.z > 0 ? 1 : -1;

	Ray rayLocal;
	rayLocal.d = ray.d / scale;

	// Lazy security, should be improved: 
	// ray should be clamped instead of totally ignored
	if ((posLocal.x < 0) || (posLocal.x >= gridWidth) 
		|| (posLocal.y < 0) || (posLocal.y >= gridWidth) 
		|| (posLocal.z < 0) || (posLocal.z >= gridWidth))
	{
		return vec4(-4.f, 0, 0, 0);
	}

	ivec3 cellBoundary = a;
	if (dir.x > 0) ++cellBoundary.x;
	if (dir.y > 0) ++cellBoundary.y;
	if (dir.z > 0) ++cellBoundary.z;

	ivec3 bounds;
	bounds.x = dir.x > 0 ? int(gridWidth) : -1;
	bounds.y = dir.y > 0 ? int(gridWidth) : -1;
	bounds.z = dir.z > 0 ? int(gridWidth) : -1;

	//ivec3 error = d;
	//int range = d.x + d.y + d.z;

	vec3 tmax = (vec3(cellBoundary) - va) * invDir;
	const vec3 tdelta = step * invDir;

	while (true)
	{
		const uint gridIdx = posLocal.z * gridWidth * gridWidth + posLocal.y * gridWidth + posLocal.x;
		const uint voxelPtr = GetBroadphaseCell(gridIdx);

		if (voxelPtr != 0)
		{
			//norm = vec3(1, 0, 0);
			//return vec4(0.1, 0, 0, 1);
			vec3 normal;
			rayLocal.o = va - posLocal;
			vec4 result = CastLocal(rayLocal, voxelPtr, drawDepth, normal);
			if (result.x > 0)
			{
				norm = normal;
				return result;
			}
		}

		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				posLocal.x += step.x;
				if (posLocal.x == bounds.x) break;
				tmax.x += tdelta.x;
			}
			else
			{
				posLocal.z += step.z;
				if (posLocal.z == bounds.z) break;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				posLocal.y += step.y;
				if (posLocal.y == bounds.y) break;
				tmax.y += tdelta.y;
			}
			else
			{
				posLocal.z += step.z;
				if (posLocal.z == bounds.z) break;
				tmax.z += tdelta.z;
			}
		}
	}

	return vec4(-4.f, 0, 0, 0);
}
#endif