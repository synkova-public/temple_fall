// Header
#include "screen.hpp"

#include <cmath>

Texture gScreen::screen_texture_title_selected_start;
Texture gScreen::screen_texture_title_selected_lore;
Texture gScreen::screen_texture_title_selected_score;
Texture gScreen::screen_texture_title_selected_exit;

Texture gScreen::screen_texture_lore_selected_return;
Texture gScreen::screen_texture_score_selected_return;

Texture gScreen::screen_texture_death_retry;
Texture gScreen::screen_texture_death_menu;

bool gScreen::init()
{
    // Load shared texture
    if (!screen_texture_title_selected_start.is_valid())
    {
        if (!screen_texture_title_selected_start.load_from_file(textures_path("screen_title_selected_start.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_title_selected_lore.is_valid())
    {
        if (!screen_texture_title_selected_lore.load_from_file(textures_path("screen_title_selected_lore.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_title_selected_score.is_valid())
    {
        if (!screen_texture_title_selected_score.load_from_file(textures_path("screen_title_selected_score.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_title_selected_exit.is_valid())
    {
        if (!screen_texture_title_selected_exit.load_from_file(textures_path("screen_title_selected_exit.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_lore_selected_return.is_valid())
    {
        if (!screen_texture_lore_selected_return.load_from_file(textures_path("screen_lore_selected_return.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_score_selected_return.is_valid())
    {
        if (!screen_texture_score_selected_return.load_from_file(textures_path("screen_score_selected_return.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_death_retry.is_valid())
    {
        if (!screen_texture_death_retry.load_from_file(textures_path("screen_death_retry.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    if (!screen_texture_death_menu.is_valid())
    {
        if (!screen_texture_death_menu.load_from_file(textures_path("screen_death_menu.png"), true))
        {
            fprintf(stderr, "Failed to load screen texture!");
            return false;
        }
    }

    // The position corresponds to the center of the texture.
    float wr = screen_texture_title_selected_start.width * 0.5f;
    float hr = screen_texture_title_selected_start.height * 0.5f;

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
    if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
        return false;

    motion.position = { 300.0f, 400.0f };
    motion.radians = 0.f;
    motion.speed = 0.f;

    // Setting initial values, scale is negative to make it face the opposite way
    // 1.0 would be as big as the original texture.
    physics.scale = { 1.00f, 1.00f };

    width_scaled = screen_texture_title_selected_start.width * 1.00f;
    width_unscaled = screen_texture_title_selected_start.width;

    height_scaled = screen_texture_title_selected_start.height * 1.00f;
    height_unscaled = screen_texture_title_selected_start.height;

    displayed_screen = 0;

    return true;
}

// Releases all graphics resources
void gScreen::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void gScreen::update(float ms)
{
    // Move screen along -X based on how much time has passed, this is to (partially) avoid
    // having entities move at different speed based on the machine.
    float step = -1.0 * motion.speed * (ms / 1000);
    motion.position.x += step;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // HANDLE FISH AI HERE
    // DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
    // You will likely want to write new functions and need to create
    // new data structures to implement a more sophisticated Screen AI.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void gScreen::draw(const mat3& projection)
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

    if (displayed_screen == 0)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_title_selected_start.id);
    }
    else if (displayed_screen == 1)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_title_selected_lore.id);
    }
    else if (displayed_screen == 2)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_title_selected_score.id);
    }
    else if (displayed_screen == 3)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_title_selected_exit.id);
    }
    else if (displayed_screen == 4)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_lore_selected_return.id);
    }
    else if (displayed_screen == 5)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_score_selected_return.id);
    }
    else if (displayed_screen == 6)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_death_retry.id);
    }
    else if (displayed_screen == 7)
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_death_menu.id);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, screen_texture_title_selected_start.id);
    }

    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)& transform.out);
    float color[] = { 1.f, 1.f, 1.f };
    glUniform3fv(color_uloc, 1, color);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)& projection);

    // Drawing!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 gScreen::get_position() const
{
    return motion.position;
}

void gScreen::set_position(vec2 position)
{
    motion.position = position;
}

vec2 gScreen::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the screen
    // fabs is to avoid negative scale due to the facing direction.
    return { std::fabs(physics.scale.x) * screen_texture_title_selected_start.width, std::fabs(physics.scale.y) * screen_texture_title_selected_start.height };
}
