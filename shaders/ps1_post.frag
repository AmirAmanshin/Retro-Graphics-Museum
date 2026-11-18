#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float dangerIntensity;
uniform float time;

float bayer4x4(vec2 pos) {
    vec2 p = mod(floor(pos), 4.0);
    float a = mod(p.x + p.y * 2.0, 4.0) * 4.0;
    float b = mod(p.x * 2.0 + p.y, 4.0);
    return ((a + b) / 16.0) - 0.5;
}

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec2 ps1Res = vec2(240.0, 180.0);
    vec2 ps1Pixel = floor(TexCoords * ps1Res);
    vec2 snappedUV = (ps1Pixel + 0.5) / ps1Res;

    float aberration = 0.0022 + dangerIntensity * 0.0045;
    float r = texture(screenTexture, snappedUV + vec2(aberration, 0.0)).r;
    float g = texture(screenTexture, snappedUV).g;
    float b = texture(screenTexture, snappedUV - vec2(aberration, 0.0)).b;
    vec3 col = vec3(r, g, b);

    vec2 uv = snappedUV * 2.0 - 1.0;
    float vig = 1.0 - dot(uv, uv) * 0.42;
    col *= clamp(vig, 0.15, 1.0);

    if (dangerIntensity > 0.01) {
        float pulse = 0.65 + 0.35 * sin(time * 10.0);
        float edge = clamp(dot(uv, uv) * 0.65, 0.0, 1.0);
        col += vec3(0.55, 0.02, 0.02) * edge * dangerIntensity * pulse;
    }

    float noise = (hash12(ps1Pixel + fract(time) * 100.0) - 0.5) * (0.045 + dangerIntensity * 0.04);
    col += noise;

    float colorSteps = 10.0;
    float dither = bayer4x4(ps1Pixel) * (1.15 / colorSteps);
    col = floor(clamp(col + dither, 0.0, 1.0) * colorSteps + 0.5) / colorSteps;

    float scanline = 0.84 + 0.16 * sin(snappedUV.y * ps1Res.y * 3.14159265 * 2.0);
    col *= scanline;

    FragColor = vec4(col, 1.0);
}