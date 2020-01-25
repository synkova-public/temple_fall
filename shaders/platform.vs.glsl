#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out float lux;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 lightpos;
uniform float illumination;

void main()
{
	texcoord = in_texcoord;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	vec3 lightposp = projection * transform * vec3(lightpos, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
  lux = min(1 / pow(length(pos.xy - lightposp.xy) * 6, 1.2), 3);
  // lux = min(1 / pow(length(in_position.xy - lightpos.xy) * 5, 1.2), 3);
  // lux = 0.1;
  lux = illumination;
}
