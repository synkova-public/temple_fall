#version 330

// From vertex shader
in vec2 texcoord;
in vec2 fragment_lightpos;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

uniform vec3 shift;

// uniform vec2 lightpos;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
    // the following is the normal lighting
    // color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y - shift.y));

    // the following is the dynamic lighting
  float lux = max(min(1 / pow(length(vec2(texcoord.x - fragment_lightpos.x, 1.9*(texcoord.y - fragment_lightpos.y))) * 2, 1.2), 1.75), 0.85);
  color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y - shift.y)) * lux;
}
