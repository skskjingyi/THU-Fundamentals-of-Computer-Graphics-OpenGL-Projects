#version 330 core
in vec2 TexCoord;

uniform sampler2D bgTexture;
out vec4 FragColor;

void main()
{
    FragColor = texture(bgTexture, TexCoord);
}
