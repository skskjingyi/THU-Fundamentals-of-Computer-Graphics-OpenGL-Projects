#version 330 core

in vec2 texCoord;

//uniform means the variable comes from CPU
//sampler2D is a GLSL type = a regular 2D texture (PNG, JPG, etc.)
//shader variable that receive 2d texture from CPU

uniform sampler2D uTexture; 

out vec4 color;

void main()
{
  vec4 texColor = texture(uTexture, texCoord);
  // If alpha is very low, drop this fragment completely
  if (texColor.a < 0.1)
    discard;

  color = texColor;
}

