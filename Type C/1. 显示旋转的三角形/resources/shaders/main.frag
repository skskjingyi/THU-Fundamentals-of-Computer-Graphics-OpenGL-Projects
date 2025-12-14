#version 330 core

in vec3 smoothColor;
flat in vec3 flatColor; 

uniform int shadingMode;

out vec4 color;

void main()
{
  if (shadingMode == 1)
  {
    color = vec4(flatColor, 1.0);
  }
  else 
  {
    color = vec4(smoothColor, 1.0f);
  }
  
}
