// Header
#include "score_digit.hpp"

#include <cmath>

Texture Score_Digit::score_digit_texture_0;
Texture Score_Digit::score_digit_texture_1;
Texture Score_Digit::score_digit_texture_2;
Texture Score_Digit::score_digit_texture_3;
Texture Score_Digit::score_digit_texture_4;
Texture Score_Digit::score_digit_texture_5;
Texture Score_Digit::score_digit_texture_6;
Texture Score_Digit::score_digit_texture_7;
Texture Score_Digit::score_digit_texture_8;
Texture Score_Digit::score_digit_texture_9;

bool Score_Digit::init()
{
	// Load shared texture
	if (!score_digit_texture_0.is_valid())
	{
		if (!score_digit_texture_0.load_from_file(textures_path("score_digit_0.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_1.is_valid())
	{
		if (!score_digit_texture_1.load_from_file(textures_path("score_digit_1.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_2.is_valid())
	{
		if (!score_digit_texture_2.load_from_file(textures_path("score_digit_2.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_3.is_valid())
	{
		if (!score_digit_texture_3.load_from_file(textures_path("score_digit_3.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_4.is_valid())
	{
		if (!score_digit_texture_4.load_from_file(textures_path("score_digit_4.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_5.is_valid())
	{
		if (!score_digit_texture_5.load_from_file(textures_path("score_digit_5.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_6.is_valid())
	{
		if (!score_digit_texture_6.load_from_file(textures_path("score_digit_6.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_7.is_valid())
	{
		if (!score_digit_texture_7.load_from_file(textures_path("score_digit_7.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_8.is_valid())
	{
		if (!score_digit_texture_8.load_from_file(textures_path("score_digit_8.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	if (!score_digit_texture_9.is_valid())
	{
		if (!score_digit_texture_9.load_from_file(textures_path("score_digit_9.png"), true))
		{
			fprintf(stderr, "Failed to load score_digit texture!");
			return false;
		}
	}

	// The position corresponds to the center of the texture.
	float wr = score_digit_texture_0.width * 0.5f;
	float hr = score_digit_texture_0.height * 0.5f;

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
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	motion.radians = 0.f;
	motion.speed = 380.f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	physics.scale = { 0.75f, 0.75f };

	width_scaled    = score_digit_texture_0.width * 0.75;
	width_unscaled  = score_digit_texture_0.width;

	height_scaled   = score_digit_texture_0.height * 0.75;
	height_unscaled = score_digit_texture_0.height;

	m_number = 0;

	return true;
}

// Releases all graphics resources
void Score_Digit::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Score_Digit::update(float ms)
{
	// Move score_digit along -X based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	float step = -1.0 * motion.speed * (ms / 1000);
	motion.position.x += step;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE FISH AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Score_Digit AI. 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void Score_Digit::draw(const mat3& projection)
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

	if (m_number == 0)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_0.id);
	}
	else if (m_number == 1)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_1.id);
	}
	else if (m_number == 2)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_2.id);
	}
	else if (m_number == 3)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_3.id);
	}
	else if (m_number == 4)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_4.id);
	}
	else if (m_number == 5)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_5.id);
	}
	else if (m_number == 6)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_6.id);
	}
	else if (m_number == 7)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_7.id);
	}
	else if (m_number == 8)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_8.id);
	}
	else if (m_number == 9)
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_9.id);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, score_digit_texture_0.id);
	}

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.out);
	float color[] = { 1.f, 1.f, 1.f };
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Score_Digit::get_position() const
{
	return motion.position;
}

void Score_Digit::set_position(vec2 position)
{
	motion.position = position;
}

vec2 Score_Digit::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the score_digit 
	// fabs is to avoid negative scale due to the facing direction.
	return { std::fabs(physics.scale.x) * score_digit_texture_0.width, std::fabs(physics.scale.y) * score_digit_texture_0.height };
}