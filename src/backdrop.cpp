#include "backdrop.hpp"
#include <cmath>

Texture Backdrop::backdrop_texture;
float Backdrop::fallen;
float Backdrop::elapsed;

bool Backdrop::init()
{
    fallen  = 0;
    elapsed = 0;

	if (!backdrop_texture.is_valid())
	{
		if (!backdrop_texture.load_from_file(textures_path("backdrop_texture.png"), false))
		{
			fprintf(stderr, "Failed to load backdrop texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture.
	float wr = backdrop_texture.width * 0.5f;
	float hr = backdrop_texture.height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { 0.f, 1.f };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { 1.f, 1.f,  };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { 1.f, 0.f };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { 0.f, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("loop.vs.glsl"), shader_path("loop.fs.glsl")))
		return false;

	motion.radians = 0.f;
	motion.speed = 300.f;
	motion.position = {300.f, 400.f};

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	physics.scale = { 0.318f, 0.318f };

	return true;
}

// Releases all graphics resources
void Backdrop::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Backdrop::update(float ms)
{
    elapsed += ms;

    if (ms > 0.00125f)
    {
        fallen -= 0.00125f;
    }
    else
    {
        fallen -= ms;
    }
}

void Backdrop::draw(const mat3& projection )
{
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	transform.begin();
	transform.translate(motion.position);
	transform.rotate(motion.radians);
	transform.scale(physics.scale);
	transform.end();

	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
    GLint shift_uloc = glGetUniformLocation(effect.program, "shift");
    GLint lightpos_uloc = glGetUniformLocation(effect.program, "lightpos");


	// Setting vertices and indices
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backdrop_texture.id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.out);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
  float shift[] = { 0.f, fallen, 0.f };
  glUniform3fv(shift_uloc, 1, shift);
  // float lightpos[] = {0, 0.1};
  float lightpos[] = {lightsource.x / 600.f, (lightsource.y + 100) / 1000.f};
  glUniform2fv(lightpos_uloc, 1, lightpos);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Backdrop::get_position() const
{
	return motion.position;
}

void Backdrop::set_position(vec2 position)
{
	motion.position = position;
}

vec2 Backdrop::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the Backdrop
	// fabs is to avoid negative scale due to the facing direction.
	return { std::fabs(physics.scale.x) * backdrop_texture.width, std::fabs(physics.scale.y) * backdrop_texture.height };
}
