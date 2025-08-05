// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

// Utility function to get a vector perpendicular to an input vector 
//    (from "Efficient Construction of Perpendicular Vectors Without Branching")
// https://gamehacker1999.github.io/posts/SSGI/
// https://blog.selfshadow.com/2011/10/17/perp-vectors/
vec3 GetPerpendicularVector(vec3 u)
{
	vec3 a = abs(u);
	uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
	uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
	uint zm = 1 ^ (xm | ym);
	return cross(u, vec3(xm, ym, zm));
}

vec3 GetPerpendicularVector2(vec3 u)
{
    vec3 a = abs(u);
    vec3 v;
    if (a.x <= a.y && a.x <= a.z)
        v = vec3(0, -u.z, u.y);
    else if (a.y <= a.x && a.y <= a.z)
        v = vec3(-u.z, 0, u.x);
    else
        v = vec3(-u.y, u.x, 0);
    return v;
}

vec3 GetFragPosViewSpace(vec2 uv, vec2 fov, float depth)
{
    // https://gamedev.stackexchange.com/questions/108856/fast-position-reconstruction-from-depth/111885#111885
    vec2 v_fov_scale;
    v_fov_scale.x = tan(fov.x / 2.0);
    v_fov_scale.y = tan(fov.y / 2.0);
    v_fov_scale *= 2.0; // Required to avoid the multiplication by 2.0 in the fragment shader at "vec2 P_ndc = vec2(1.0) - texture_coordinate * 2.0;".
    vec2 half_ndc_position = vec2(0.5) - uv;    // No need to multiply by two, because we already baked that into "v_tan_fov.xy".
    return vec3(half_ndc_position * v_fov_scale.xy * -depth, -depth); // "-depth" because in OpenGL the camera is staring down the -z axis (and we're storing the unsigned depth).
}

vec3 GetFragPos(mat4 invViewProj, vec2 uv, float depth)
{
    vec4 wpos = invViewProj * (vec4(uv, depth, 1.0) * 2.0 - 1.0);
    return wpos.xyz / wpos.w;
    /*
    vec3 viewRay = normalize(inPos);

    // Sample the depth buffer and convert it to linear depth
    float linearDepth = projectionB / (depth - projectionA);

    // Project the view ray onto the camera's z-axis
    float viewZDist = dot(cameraAt, viewRay);

    // Scale the view ray by the ratio of the linear z value to the projected view ray
    //return  cameraPos + viewRay * (linearDepth / viewZDist);
    */
}