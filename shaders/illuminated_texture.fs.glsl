#version 330

// From vertex shader
in vec2 texcoord;
in float lux;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
  color.x = color.x * lux;
  color.y = color.y * lux;
  color.z = color.z * lux;
}
