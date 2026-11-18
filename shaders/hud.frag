#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec2 FragPosPx;

uniform int mode; // 0 = rounded box, 1 = glyph text
uniform vec2 boxPos;
uniform vec2 boxSize;
uniform float cornerRadius;
uniform vec4 fillColor;
uniform vec4 borderColor;
uniform sampler2D fontTex;
uniform vec4 textColor;

float roundedBoxSDF(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + vec2(r);
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    if (mode == 0) {
        vec2 halfSize = boxSize * 0.5;
        vec2 center = boxPos + halfSize;
        float d = roundedBoxSDF(FragPosPx - center, halfSize, cornerRadius);

        float alpha = 1.0 - smoothstep(-1.0, 1.0, d);
        if (alpha <= 0.001) discard;

        float borderMask = smoothstep(-2.2, -0.6, d);
        vec4 col = mix(fillColor, borderColor, borderMask);
        FragColor = vec4(col.rgb, col.a * alpha);
    } else {
        float glyph = texture(fontTex, TexCoords).r;
        if (glyph < 0.5) discard;
        FragColor = vec4(textColor.rgb, textColor.a * glyph);
    }
}