// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

// Atmospheric scattering constants
const float EARTH_RADIUS = 6360e3;     // meters
const float ATMOSPHERE_HEIGHT = 60e3;  // meters
const vec3 BETA_R = vec3(5.5e-6, 13.0e-6, 22.4e-6);  // Rayleigh scattering coefficient
const vec3 BETA_M = vec3(21e-6);       // Mie scattering coefficient
const float H_R = 8e3;                 // Rayleigh scale height
const float H_M = 1.2e3;               // Mie scale height
const float G = 0.76;                  // Mie scattering asymmetry factor

// Sun parameters
vec3 sunDirection = normalize(vec3(0, 0.5, 1));
float sunIntensity = 20.0; // typically between 1 and 20
float turbidity = 3.0f;

vec3 randomUnitVector(vec2 rand)
{
    float y = rand.y * 2.0 - 1.0;
    float r = sqrt(1.0 - y * y);
    float phi = 2.0 * PI * rand.x;
    return vec3(r * cos(phi), y, r * sin(phi));
}

float calculateSunPDF(vec3 sampleDir, vec3 sunDir, float coneAngle)
{
    float cosTheta = dot(sampleDir, sunDir);
    float sigma = coneAngle * coneAngle;
    return exp(-(1.0 - cosTheta) / sigma) / (2.0 * PI * sigma);
}

vec2 hash2(uvec2 xy)
{
    xy = xy * 1664525u + 1013904223u;
    xy += xy.yx * 1664525u;
    xy = xy ^ (xy >> 16u);
    xy = xy * 1664525u;
    xy += xy.yx * 1664525u;
    return vec2(xy) / float(0xFFFFFFFFu);
}

/// Computes optical depth for a ray from point at height h to top of atmosphere
float ComputeOpticalDepth(float h, float cosTheta)
{
    float r = EARTH_RADIUS + h;
    float rMu = r * cosTheta;
    float s = sqrt(EARTH_RADIUS * EARTH_RADIUS - r * r * (1.0 - cosTheta * cosTheta)) - rMu;

    // Simplified exponential approximation
    float hr = exp(-(r - EARTH_RADIUS) / H_R) / H_R;
    float hm = exp(-(r - EARTH_RADIUS) / H_M) / H_M;

    return exp(-s * (hr + hm));
}

/// Computes sky color using atmospheric scattering
vec3 ComputeSkyLight(vec3 rayDir)
{
    float h = 0.0; // Ground level
    float cosTheta = dot(rayDir, vec3(0.0, 1.0, 0.0));

    // Optical depth for view ray
    float opticalDepth = ComputeOpticalDepth(h, cosTheta);

    // Phase functions
    float cosSun = dot(rayDir, sunDirection);
    float phaseR = 3.0 / (16.0 * PI) * (1.0 + cosSun * cosSun);
    float phaseM = (3.0 / (8.0 * PI)) * ((1.0 - G * G) * (1.0 + cosSun * cosSun)) /
        ((2.0 + G * G) * pow(1.0 + G * G - 2.0 * G * cosSun, 1.5));

    // Optical depth for sun light
    float opticalDepthSun = ComputeOpticalDepth(h, dot(sunDirection, vec3(0.0, 1.0, 0.0)));

    // Total scattering
    vec3 scattering = (BETA_R * phaseR + BETA_M * phaseM) * (opticalDepth + opticalDepthSun);

    // Final sky color with sun intensity
    return sunIntensity * exp(-scattering);
}

vec3 ComputeSkyLight2(vec3 rayDir)
{
    float sunCos = dot(rayDir, sunDirection);
    float horizon = sqrt(max(0.0, rayDir.y)); // Y-up horizon fade
    vec3 sky = vec3(0.0);

    // Rayleigh scattering (blue zenith -> red horizon)
    sky += BETA_R * (1.0 + sunCos * sunCos) * horizon;

    // Mie scattering (sun glow)
    sky += BETA_M * pow(max(0.0, sunCos), 8.0) * horizon;

    return sky * sunIntensity;
}

vec3 ComputeSkyLight3(vec3 rayDir)
{
    float sunCos = dot(rayDir, sunDirection);
    float sunGlow = pow(max(0.0, sunCos), 64.0); // Sun disc
    return vec3(sunGlow); // Should show a bright spot where sun is
}

vec3 ComputeSkyLight4(vec3 rayDir)
{
    // --- Phase Functions ---
    float sunCos = dot(rayDir, sunDirection);

    // Rayleigh phase (1 + cos^2 theta)
    float phaseR = (3.0 / (16.0 * PI)) * (1.0 + sunCos * sunCos);

    // Mie phase (sharp forward-scattering for sun disk)
    float g = 0.76; // Scattering asymmetry
    float phaseM = (3.0 / (8.0 * PI)) * ((1.0 - g * g) * (1.0 + sunCos * sunCos)) /
        ((2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * sunCos, 1.5));

    // --- Scattering Terms ---
    float horizon = smoothstep(0.0, 0.2, rayDir.y); // Soft horizon fade (Y-up)

    // Rayleigh (blue sky + red horizon)
    vec3 rayleigh = BETA_R * phaseR * horizon;

    // Mie (sun disk + glow)
    vec3 mie = BETA_M * phaseM * horizon;

    // --- Combine and Scale ---
    return (rayleigh + mie) * sunIntensity * 2.0;
}

vec3 ComputeSkyLight5(vec3 dir)
{
    // Rayleigh scattering coefficients
    vec3 rayleighCoeff = vec3(5.8e-6, 13.5e-6, 33.1e-6);

    // Mie scattering coefficients
    float mieCoeff = 2.0e-5;
    float mieDir = 0.76;

    // Optical depth
    float opticalDepth = 1.0 / max(0.1, dir.y);

    // Scattering
    float cosTheta = dot(dir, sunDirection);
    float rayleighPhase = (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
    float miePhase = (3.0 / (8.0 * PI)) *
        ((1.0 - mieDir * mieDir) * (1.0 + cosTheta * cosTheta)) /
        ((2.0 + mieDir * mieDir) * pow(1.0 + mieDir * mieDir - 2.0 * mieDir * cosTheta, 1.5));

    // Combine
    vec3 skyColor = (rayleighCoeff * rayleighPhase + mieCoeff * miePhase) * opticalDepth;

    // Add sun
    float sun = smoothstep(0.998, 0.999, cosTheta);
    skyColor += vec3(1.0, 0.9, 0.8) * sun * 100.0;

    return skyColor;
}

// Returns the visual color of the sky in a given direction
vec3 GetSkyColor(vec3 dir)
{
    // Basic parameters
    float sunHeight = max(0.0, sunDirection.y);
    float viewHeight = max(0.01, dir.y);
    float cosTheta = dot(dir, sunDirection);

    // Rayleigh scattering coefficients (wavelength dependent)
    vec3 rayleighCoeff = vec3(
        5.8e-6 * (2.0 - turbidity * 0.5),
        13.5e-6 * (2.0 - turbidity * 0.5),
        33.1e-6 * (2.0 - turbidity * 0.5)
    );

    // Mie scattering (aerosols)
    float mieCoeff = 2.0e-5 * turbidity;
    float mieDir = 0.76 + turbidity * 0.04;

    // Optical depth (thicker atmosphere near horizon)
    float rayleighDepth = exp(-0.25 / viewHeight);
    float mieDepth = exp(-0.0025 / viewHeight);

    // Phase functions
    float rayleighPhase = 0.0596831 * (1.0 + cosTheta * cosTheta); 

    float miePhaseNum = (1.0 - mieDir * mieDir) * (1.0 + cosTheta * cosTheta);
    float miePhaseDenom = (2.0 + mieDir * mieDir) * pow(1.0 + mieDir * mieDir - 2.0 * mieDir * cosTheta, 1.5);
    float miePhase = 0.119366 * miePhaseNum / miePhaseDenom;

    // Sun disk
    float sun = smoothstep(0.9995, 0.9999, cosTheta) * sunIntensity;

    // Combine components
    vec3 color = rayleighCoeff * rayleighPhase * rayleighDepth
        + vec3(mieCoeff) * miePhase * mieDepth
        + vec3(1.0, 0.9, 0.7) * sun;

    /*
    // Horizon darkening
    float horizon = 1.0 - smoothstep(0.0, 0.2, viewHeight);
    color *= 1.0 - horizon * 0.5;

    // Ground color when looking down
    if (dir.y < 0.0)
    {
        vec3 groundAlbedo = vec3(0.3);
        color = groundAlbedo * (color + vec3(sun) * 0.1);
    }
    */

    return color;
}

vec3 GetSkyLight(vec3 dir)
{
    // Basic parameters
    float viewHeight = max(0.01, dir.y);
    float cosTheta = dot(dir, sunDirection);

    // Scattering coefficients
    vec3 rayleighCoeff = vec3(
        5.8e-6 * (2.0 - turbidity * 0.5),
        13.5e-6 * (2.0 - turbidity * 0.5),
        33.1e-6 * (2.0 - turbidity * 0.5)
    );

    float mieCoeff = 2.0e-5 * turbidity;
    float mieDir = 0.76 + turbidity * 0.04;

    // Optical depth
    float rayleighDepth = exp(-0.25 / viewHeight);
    float mieDepth = exp(-0.0025 / viewHeight);

    // Phase functions
    float rayleighPhase = 0.0596831 * (1.0 + cosTheta * cosTheta);

    float miePhaseNum = (1.0 - mieDir * mieDir) * (1.0 + cosTheta * cosTheta);
    float miePhaseDenom = (2.0 + mieDir * mieDir) * pow(1.0 + mieDir * mieDir - 2.0 * mieDir * cosTheta, 1.5);
    float miePhase = 0.119366 * miePhaseNum / miePhaseDenom;

    // Combine scattering terms (excluding sun disk)
    vec3 light = rayleighCoeff * rayleighPhase * rayleighDepth
        + vec3(mieCoeff) * miePhase * mieDepth;

    // Convert to luminance (CIE XYZ weights)
    float luminance = light.r * 0.2126 + light.g * 0.7152 + light.b * 0.0722;

    // Scale by sun position (brighter when sun is high)
    float sunHeight = max(0.0, sunDirection.y);
    luminance *= 1.0 + sunHeight * sunIntensity * 0.5;

    // Return as uniform white light (or keep colored if preferred)
    return vec3(luminance);
}

vec3 SampleSkyLight0(vec3 pos, vec3 normal)
{
    ivec2 ipos = ivec2(gl_GlobalInvocationID);
    int texnum = _globalUBO._currentFrameIdx;
    ivec2 texpos = ipos & ivec2(BLUE_NOISE_RES - 1);
    float jitter_x = texelFetch(TEX_BLUE_NOISE, ivec3(texpos, (texnum + 0) & (NUM_BLUE_NOISE_TEX - 1)), 0).r;
    float jitter_y = texelFetch(TEX_BLUE_NOISE, ivec3(texpos, (texnum + 1) & (NUM_BLUE_NOISE_TEX - 1)), 0).r;
    vec2 rand = vec2(jitter_x, jitter_y);

    float phi = 2.0 * PI * rand.x;
    float cosTheta = sqrt(rand.y);  // Importance sampling for cosine
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // Build orthonormal basis around normal
    vec3 tangent = normalize(cross(normal, vec3(1.0, 0.0, 0.0)));
    vec3 bitangent = cross(normal, tangent);
    vec3 sampleDir = normalize(
        sinTheta * cos(phi) * tangent +
        sinTheta * sin(phi) * bitangent +
        cosTheta * normal
    );

    /*
    // Shadow ray
    Ray r;
    r.o = pos;
    r.d = sampleDir;
    r._maxDist = maxTraceDist;

    CastResult result;
    CastGlobal(r, result);
    if (result._t < r._maxDist)
    {
        return vec3(0, 0, 0);
    }*/

    // Get sky color and divide by PDF (cosTheta / PI)
    return GetSkyLight(sampleDir) * (PI / max(cosTheta, 1e-5));
}

vec2 GetRand2()
{
    ivec2 ipos = ivec2(gl_GlobalInvocationID);
    int texnum = _globalUBO._currentFrameIdx;
    ivec2 texpos = ipos & ivec2(BLUE_NOISE_RES - 1);
    float jitter_x = texelFetch(TEX_BLUE_NOISE, ivec3(texpos, (texnum + 0) & (NUM_BLUE_NOISE_TEX - 1)), 0).r;
    float jitter_y = texelFetch(TEX_BLUE_NOISE, ivec3(texpos, (texnum + 1) & (NUM_BLUE_NOISE_TEX - 1)), 0).r;
    return vec2(jitter_x, jitter_y);
}

vec3 SampleSkyLight1(vec3 pos, vec3 normal)
{
    vec2 rand = GetRand2();

    float phi = 2.0 * PI * rand.x;
    float cosTheta = sqrt(rand.y);  // Importance sampling for cosine
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // Build orthonormal basis around normal
    vec3 tangent = normalize(cross(normal, vec3(1.0, 0.0, 0.0)));
    vec3 bitangent = cross(normal, tangent);
    vec3 sampleDir = normalize(
        sinTheta * cos(phi) * tangent +
        sinTheta * sin(phi) * bitangent +
        cosTheta * normal
    );

    /*
    // Shadow ray
    Ray r;
    r.o = pos;
    r.d = sampleDir;
    r._maxDist = maxTraceDist;

    CastResult result;
    CastGlobal(r, result);
    if (result._t < r._maxDist)
    {
        return vec3(0, 0, 0);
    }*/

    // Get sky color and divide by PDF (cosTheta / PI)
    return GetSkyLight(sampleDir) * (PI / max(cosTheta, 1e-5));
}

vec3 RandomSunJitter(vec2 rand, vec3 sunDir, float coneAngle)
{
    vec3 tangent = normalize(cross(sunDir, vec3(0, 1, 0)));
    vec3 bitangent = cross(sunDir, tangent);
    float r = sqrt(rand.x) * coneAngle;
    float phi = 2.0 * PI * rand.y;
    return normalize(sunDir + r * (cos(phi) * tangent + sin(phi) * bitangent));
}

vec3 SampleSkyLight(vec3 pos, vec3 normal)
{
    vec2 rand = GetRand2();

    // --- 1. Blend between sun direction and cosine sampling ---
    float sunWeight = 0.5; // 50% chance to sample toward sun (adjust as needed)
    bool sampleSun = (rand.x < sunWeight);
    if (sampleSun)
    {
        if (dot(normal, sunDirection) > 0.0f)
        {
            // --- 2. Importance sample near sun direction ---
            float coneAngle = 0.05; // Controls sun sampling tightness
            vec3 sunSampleDir = RandomSunJitter(rand, sunDirection, coneAngle);

            // Calculate PDF for sun sampling (directional bias)
            float sunPdf = sunWeight * calculateSunPDF(sunSampleDir, sunDirection, coneAngle);
            return GetSkyLight(sunSampleDir) / max(sunPdf, 1e-5);
        }
    }

    // --- 3. Fallback to cosine-weighted sampling ---
    float phi = 2.0 * PI * rand.y;
    float cosTheta = sqrt(1.0 - rand.y); // Cosine-weighted
    float sinTheta = sqrt(rand.y);

    // Build orthonormal basis around normal (Y-up)
    vec3 tangent = normalize(cross(normal, vec3(1.0, 0.0, 0.0)));
    vec3 bitangent = cross(normal, tangent);
    vec3 sampleDir = normalize(
        sinTheta * cos(phi) * tangent +
        sinTheta * sin(phi) * bitangent +
        cosTheta * normal
    );

    // PDF for cosine sampling (adjusted for blend)
    float pdf = (1.0 - sunWeight) * (cosTheta / PI);
    return GetSkyLight(sampleDir) / max(pdf, 1e-5);
}