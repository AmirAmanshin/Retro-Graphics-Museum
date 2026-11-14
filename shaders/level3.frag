#version 330 core
in vec2 TexCoord;
in float Depth;

out vec4 FragColor;

uniform sampler2D texWall1;
uniform sampler2D texWall2;
uniform sampler2D texWall3;
uniform sampler2D texExit;
uniform int texIndex;

void main() {
    vec4 texColor;
    if (texIndex == 0) texColor = texture(texWall1, TexCoord);
    else if (texIndex == 1) texColor = texture(texWall2, TexCoord);
    else if (texIndex == 2) texColor = texture(texWall3, TexCoord);
    else texColor = texture(texExit, TexCoord);

    float fogFactor = clamp((9.0 - Depth) / (9.0 - 1.5), 0.1, 1.0);
    vec3 fogColor = vec3(0.05, 0.05, 0.07);
    FragColor = vec4(mix(fogColor, texColor.rgb, fogFactor), 1.0);
}