#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D texWall1;
uniform sampler2D texWall2;
uniform sampler2D texWall3;
uniform sampler2D texExit;

void main() {
    if (TexIndex == 0)      FragColor = texture(texWall1, TexCoord);
    else if (TexIndex == 1) FragColor = texture(texWall2, TexCoord);
    else if (TexIndex == 2) FragColor = texture(texWall3, TexCoord);
    else if (TexIndex == 3) FragColor = texture(texExit, TexCoord);
    else                    FragColor = texture(texWall1, TexCoord);
}