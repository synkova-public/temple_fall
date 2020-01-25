// Header
#include "score_label.hpp"

#include <cmath>

Texture Score_Label::score_label_texture;

Texture Score_Label::high_score_1_show_texture;
Texture Score_Label::high_score_2_show_texture;
Texture Score_Label::high_score_3_show_texture;
Texture Score_Label::high_score_4_show_texture;
Texture Score_Label::high_score_5_show_texture;

Texture Score_Label::rank1_texture;
Texture Score_Label::rank2_texture;
Texture Score_Label::rank3_texture;
Texture Score_Label::rank4_texture;
Texture Score_Label::rank5_texture;
Texture Score_Label::no_rank_texture;

bool Score_Label::init()
{
    high_score_show = false;
    high_score_numb = 0;
    death_screen_rank_show = false;

    // Load shared texture
    if (!score_label_texture.is_valid())
    {
        if (!score_label_texture.load_from_file(textures_path("score_label.png"), true))
        {
            fprintf(stderr, "Failed to load score_label texture!");
            return false;
        }
    }

    if (!high_score_1_show_texture.is_valid())
    {
        if (!high_score_1_show_texture.load_from_file(textures_path("high_score_1_show_alt.png"), true))
        {
            fprintf(stderr, "Failed to load high_score_1_show texture!");
            return false;
        }
    }

    if (!high_score_2_show_texture.is_valid())
    {
        if (!high_score_2_show_texture.load_from_file(textures_path("high_score_2_show_alt.png"), true))
        {
            fprintf(stderr, "Failed to load high_score_2_show texture!");
            return false;
        }
    }

    if (!high_score_3_show_texture.is_valid())
    {
        if (!high_score_3_show_texture.load_from_file(textures_path("high_score_3_show_alt.png"), true))
        {
            fprintf(stderr, "Failed to load high_score_3_show texture!");
            return false;
        }
    }

    if (!high_score_4_show_texture.is_valid())
    {
        if (!high_score_4_show_texture.load_from_file(textures_path("high_score_4_show_alt.png"), true))
        {
            fprintf(stderr, "Failed to load high_score_4_show texture!");
            return false;
        }
    }

    if (!high_score_5_show_texture.is_valid())
    {
        if (!high_score_5_show_texture.load_from_file(textures_path("high_score_5_show_alt.png"), true))
        {
            fprintf(stderr, "Failed to load high_score_5_show texture!");
            return false;
        }
    }

    if (!rank1_texture.is_valid())
    {
        if (!rank1_texture.load_from_file(textures_path("achievement_1.png"), true))
        {
            fprintf(stderr, "Failed to load achievement_1 texture!");
            return false;
        }
    }
    if (!rank2_texture.is_valid())
    {
        if (!rank2_texture.load_from_file(textures_path("achievement_2.png"), true))
        {
            fprintf(stderr, "Failed to load achievement_2 texture!");
            return false;
        }
    }
    if (!rank3_texture.is_valid())
    {
        if (!rank3_texture.load_from_file(textures_path("achievement_3.png"), true))
        {
            fprintf(stderr, "Failed to load achievement_3 texture!");
            return false;
        }
    }
    if (!rank4_texture.is_valid())
    {
        if (!rank4_texture.load_from_file(textures_path("achievement_4.png"), true))
        {
            fprintf(stderr, "Failed to load achievement_4 texture!");
            return false;
        }
    }
    if (!rank5_texture.is_valid())
    {
        if (!rank5_texture.load_from_file(textures_path("achievement_5.png"), true))
        {
            fprintf(stderr, "Failed to load achievement_5 texture!");
            return false;
        }
    }
    if (!no_rank_texture.is_valid())
    {
        if (!no_rank_texture.load_from_file(textures_path("achievement_none.png"), true))
        {
            fprintf(stderr, "Failed to load achievement_none texture!");
            return false;
        }
    }

    // The position corresponds to the center of the texture.
    float wr = score_label_texture.width * 0.5f;
    float hr = score_label_texture.height * 0.5f;

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

    motion.radians = 0.f;
    motion.speed = 380.f;

    // Setting initial values, scale is negative to make it face the opposite way
    // 1.0 would be as big as the original texture.
    physics.scale = { 0.75f, 0.75f };

    width_scaled = score_label_texture.width * 0.75;
    width_unscaled = score_label_texture.width;

    height_scaled = score_label_texture.height * 0.75;
    height_unscaled = score_label_texture.height;

    return true;
}

// Releases all graphics resources
void Score_Label::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Score_Label::update(float ms)
{
    // Move score_label along -X based on how much time has passed, this is to (partially) avoid
    // having entities move at different speed based on the machine.
    float step = -1.0 * motion.speed * (ms / 1000);
    motion.position.x += step;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // HANDLE FISH AI HERE
    // DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
    // You will likely want to write new functions and need to create
    // new data structures to implement a more sophisticated Score_Label AI.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void Score_Label::draw(const mat3& projection)
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

    if (!high_score_show && !death_screen_rank_show)
    {
        glBindTexture(GL_TEXTURE_2D, score_label_texture.id);
    }
    else if (death_screen_rank_show)
    {
        if (death_screen_rank == 1)
        {
            glBindTexture(GL_TEXTURE_2D, rank1_texture.id);
        }
        else if (death_screen_rank == 2)
        {
            glBindTexture(GL_TEXTURE_2D, rank2_texture.id);
        }
        else if (death_screen_rank == 3)
        {
            glBindTexture(GL_TEXTURE_2D, rank3_texture.id);
        }
        else if (death_screen_rank == 4)
        {
            glBindTexture(GL_TEXTURE_2D, rank4_texture.id);
        }
        else if (death_screen_rank == 5)
        {
            glBindTexture(GL_TEXTURE_2D, rank5_texture.id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, no_rank_texture.id);
        }
    }
    else
    {
        if (high_score_numb == 1)
        {
            glBindTexture(GL_TEXTURE_2D, high_score_1_show_texture.id);
        }
        else if (high_score_numb == 2)
        {
            glBindTexture(GL_TEXTURE_2D, high_score_2_show_texture.id);
        }
        else if (high_score_numb == 3)
        {
            glBindTexture(GL_TEXTURE_2D, high_score_3_show_texture.id);
        }
        else if (high_score_numb == 4)
        {
            glBindTexture(GL_TEXTURE_2D, high_score_4_show_texture.id);
        }
        else if (high_score_numb == 5)
        {
            glBindTexture(GL_TEXTURE_2D, high_score_5_show_texture.id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, high_score_1_show_texture.id);
        }
    }

    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)& transform.out);
    float color[] = { 1.f, 1.f, 1.f };
    glUniform3fv(color_uloc, 1, color);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)& projection);

    // Drawing!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Score_Label::get_position() const
{
    return motion.position;
}

void Score_Label::set_position(vec2 position)
{
    motion.position = position;
}

vec2 Score_Label::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the score_label
    // fabs is to avoid negative scale due to the facing direction.
    return { std::fabs(physics.scale.x) * score_label_texture.width, std::fabs(physics.scale.y) * score_label_texture.height };
}

void Score_Label::set_scale(vec2 scale) {
    physics.scale = scale;
}
