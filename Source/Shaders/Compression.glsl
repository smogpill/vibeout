// Copyright(c) 2022 Jounayd Id Salah

// Octahedron Normal Vectors
// [Cigolle 2014, "A Survey of Efficient Representations for Independent Unit Vectors"]
//						Mean	Max
// oct		8:8			0.33709 0.94424
// snorm	8:8:8		0.17015 0.38588
// oct		10:10		0.08380 0.23467
// snorm	10:10:10	0.04228 0.09598
// oct		12:12		0.02091 0.05874

vec3 PackNormal(vec3 v)
{
	return v * 0.5 + 0.5;
}

vec3 UnpackNormal(vec3 v)
{
	return v * 2 - 1;
}

/*
vec2 UnitVectorToOctahedron(vec3 v)
{
	v.xy /= dot(vec3(1), abs(v));
	if (v.z <= 0)
		v.xy = (1 - abs(v.yx)) * (v.xy >= 0 ? vec2(1,1) : vec2(-1,-1));
	return v.xy;
}
*/

/*
vec3 OctahedronToUnitVector(vec2 oct)
{
	vec3 v = vec3(oct, 1 - dot(1, abs(oct)));
	if (N.z < 0)
		v.xy = (1 - abs(v.yx)) * (v.xy >= 0 ? vec2(1,1) : vec2(-1,-1));
	return normalize(v);
}
*/

vec2 UnitVectorToHemiOctahedron(vec3 v)
{
	v.xy /= dot(vec3(1), abs(v));
	return vec2(v.x + v.y, v.x - v.y);
}

vec3 HemiOctahedronToUnitVector(vec2 oct)
{
	oct = vec2(oct.x + oct.y, oct.x - oct.y) * 0.5;
	vec3 v = vec3(oct, vec2(1) - dot(vec2(1), abs(oct)));
	return normalize(v);
}

// Octahedron normal encode
vec2 EncodeNormal(in vec3 nor)
{
    nor /= ( abs( nor.x ) + abs( nor.y ) + abs( nor.z ) );
    nor.xy = (nor.z >= 0.0) ? nor.xy : (1.0-abs(nor.yx))*sign(nor.xy);
    vec2 v = 0.5 + 0.5*nor.xy;

    return v;
}

vec3 DecodeNormal(in vec2 v)
{
    v = -1.0 + 2.0*v;
    // Rune Stubbe's version, much faster than original
    vec3 nor = vec3(v, 1.0 - abs(v.x) - abs(v.y));
    float t = max(-nor.z,0.0);
    nor.x += (nor.x>0.0)?-t:t;
    nor.y += (nor.y>0.0)?-t:t;
    return normalize( nor );
}

uint EncodeNormal32(vec3 normal)
{
    // Project the sphere onto the octahedron (|x|+|y|+|z| = 1) and then onto the xy-plane
    float invL1Norm = 1.0 / (abs(normal.x) + abs(normal.y) + abs(normal.z));
    vec2 p = normal.xy * invL1Norm;

    // Wrap the octahedral faces from the negative-Z space
    p = (normal.z < 0) ? (1.0 - abs(p.yx)) * mix(vec2(-1.0), vec2(1.0), greaterThanEqual(p.xy, vec2(0))) : p;

    // Convert to [0..1]
    p = clamp(p.xy * 0.5 + 0.5, vec2(0), vec2(1));

    // Encode as RG16_UNORM
    uvec2 u = uvec2(p * 0xffffu);
    return u.x | (u.y << 16);
}

vec3 DecodeNormal32(uint enc)
{
    // Decode RG16_UNORM
    uvec2 u = uvec2(enc & 0xffffu, enc >> 16);
    vec2 p = vec2(u) / float(0xffff);

    // Convert to [-1..1]
    p = p * 2.0 - 1.0;

    // Decode the octahedron
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3(p.x, p.y, 1.0 - abs(p.x) - abs(p.y));
    float t = max(0, -n.z);
    n.xy += mix(vec2(t), vec2(-t), greaterThanEqual(n.xy, vec2(0)));

    return normalize(n);
}