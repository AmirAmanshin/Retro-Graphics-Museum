#version 330 core
in vec2 TexCoord;
in float Depth;

out vec4 FragColor;

uniform sampler2D spriteTexture;

void main() {
    vec4 texColor = texture(spriteTexture, TexCoord);
    if (texColor.a < 0.1)
        discard;

    float fogFactor = clamp((9.0 - Depth) / (9.0 - 1.5), 0.1, 1.0);
    vec3 fogColor = vec3(0.05, 0.05, 0.07);
    FragColor = vec4(mix(fogColor, texColor.rgb, fogFactor), texColor.a);
}