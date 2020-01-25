#version 330

// From vertex shader
in vec2 texcoord;
in float lux;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
  // float lux = min(1 / pow(length(texcoord.xy - fragment_lightpos) * 8, 1.2), 3);
	color = fcolor * texture(sampler0, vec2(texcoord.x, texcoord.y));
  color.x = color.x * lux;
  color.y = color.y * lux;
  color.z = color.z * lux;
}
