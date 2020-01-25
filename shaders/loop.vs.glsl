#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;


// Passed to fragment shader
out vec2 texcoord;
out vec2 fragment_lightpos;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform vec2 lightpos;

void main()
{
    texcoord = in_texcoord - vec2(0.0, 0.5);
    vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
    // vec3 lightpos = projection * transform * vec3(lightpos.xy, 1.0);
    // fragment_lightpos = lightpos.xy;
    fragment_lightpos = lightpos - vec2(0, 0.5);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
