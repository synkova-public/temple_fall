// Header
#include "platform.hpp"

#include <cmath>

Texture Platform::platform_texture;
Texture Platform::tar_texture;
Texture Platform::lava_texture;
Texture Platform::ghost_texture;

bool Platform::init()
{
    // Load shared texture
    if (!platform_texture.is_valid())
    {
        if (!platform_texture.load_from_file(textures_path("platform_texture.png"), true))
        {
            fprintf(stderr, "Failed to load platform texture!");
            return false;
        }
    }

    if (!tar_texture.is_valid())
    {
        if (!tar_texture.load_from_file(textures_path("platform_texture_spring.png"), true))
        {
            fprintf(stderr, "Failed to load tar platform texture!");
            return false;
        }
    }

    if (!lava_texture.is_valid())
    {
        if (!lava_texture.load_from_file(textures_path("platform_texture_lava.png"), true))
        {
            fprintf(stderr, "Failed to load lava platform texture!");
            return false;
        }
    }

    if (!ghost_texture.is_valid())
    {
        if (!ghost_texture.load_from_file(textures_path("platform_texture_ghost.png"), true))
        {
            fprintf(stderr, "Failed to load ghost platform texture!");
            return false;
        }
    }

    // The position corresponds to the center of the texture.
    float wr = platform_texture.width * 0.5f;
    float hr = platform_texture.height * 0.5f;


    TexturedVertex vertices[4];
    vertices[0].position = { -wr, +hr, -0.01f };
    vertices[0].texcoord = { 0.f, 1.f };
    vertices[1].position = { +wr, +hr, -0.01f };
    vertices[1].texcoord = { 1.f, 1.f, };
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
    if (!effect.load_from_file(shader_path("platform.vs.glsl"), shader_path("platform.fs.glsl")))
        return false;

    motion.radians = 0.f;
    motion.speed = 300.f;

    // Setting initial values, scale is negative to make it face the opposite way
    // 1.0 would be as big as the original texture.
    physics.scale = { 0.075f, 0.075f };
    type = 0;
    alpha = 2.f;
    used = false;

    return true;
}

// Releases all graphics resources
void Platform::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Platform::update(float ms)
{
    // Move Platform along -X based on how much time has passed, this is to (partially) avoid
    // having entities move at different speed based on the machine.
    float step = -1.0 * motion.speed * (ms / 1000);
    motion.position.y += step;

    if (type == 3)
    {
        if (used)
        {
            alpha -= 0.035f;
        }
        else
        {
            if (alpha < 0.30f)
            {
                alpha += 0.000f;
            }
            else
            {
                alpha += 0.035f;
            }
        }

        if (alpha < 0.30f)
        {
            alpha = 0.25f;
        }

        if (alpha > 1.00f)
        {
            alpha = 1.00f;
        }
    }

    used = false;
}

void Platform::draw(const mat3& projection)
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
    /*glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, platform_texture.id);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, tar_texture.id); */
    glActiveTexture(GL_TEXTURE0);
    if (type == 0) {
        glBindTexture(GL_TEXTURE_2D, platform_texture.id);
    }
    else if (type == 1) {
        glBindTexture(GL_TEXTURE_2D, tar_texture.id);
    }
    else if (type == 2) {
        glBindTexture(GL_TEXTURE_2D, lava_texture.id);
    }
    else if (type == 3) {
        glBindTexture(GL_TEXTURE_2D, ghost_texture.id);
    }



    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)& transform.out);
    float color[] = { 1.f, 1.f, 1.f, 1.f};
    if (alpha > 1.f) {
        color[3] = 1.f;
    } else if (alpha < 0.f) {
        color[3] = 0.f;
    } else {
        color[3] = alpha;
    }
    glUniform4fv(color_uloc, 1, color);

    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)& projection);

    GLint lightpos_uloc = glGetUniformLocation(effect.program, "lightpos");
    // float lightpos[] = {lightsource.x / 600.f, (lightsource.y + 100) / 1000.f};
    float lightpos[] = {lightsource.x - 300, lightsource.y};
    glUniform2fv(lightpos_uloc, 1, lightpos);

    GLint illumination_uloc = glGetUniformLocation(effect.program, "illumination");
    glUniform1f(illumination_uloc, illumination);
    // Drawing!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Platform::get_position() const
{
    return motion.position;
}

void Platform::set_position(vec2 position)
{
    motion.position = position;
}

vec2 Platform::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the Platform
    // fabs is to avoid negative scale due to the facing direction.
    return { std::fabs(physics.scale.x) * platform_texture.width, std::fabs(physics.scale.y) * platform_texture.height };
}