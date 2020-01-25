// Header
#include "raider.hpp"
#include "world.hpp"

// internal
#include "platform.hpp"

// stlib
#include <string>
#include <algorithm>

Texture Raider::raider_right_texture;
Texture Raider::raider_left_texture;


bool Raider::init() {
    m_vertices.clear();
    m_indices.clear();

    candown = true;
    canup = true;
    canleft = true;
    canright = true;

    x_velocity = 0.0f;
    y_velocity = 0.f;
    s_direction = 1;
    speed = 6.5f;
    jump_speed = 10.f;
    rotation_speed = 0.f;
    on_platform = false;
    walking_loop = 500;
    m_jump = false;
    walk_current_frame = 0;

    time_spent_on_lava = 0;
    coin_current = 0;
    coin_collected_so_far = 0;
    // Load shared texture
    if (!raider_right_texture.is_valid())
    {
        if (!raider_right_texture.load_from_file(textures_path("explorer_side1_torch.png"), true))
        {
            fprintf(stderr, "Failed to load raider_right texture!");
            return false;
        }
    }

    if (!raider_left_texture.is_valid())
    {
        if (!raider_left_texture.load_from_file(textures_path("explorer_side2_torch.png"), true))
        {
            fprintf(stderr, "Failed to load raider_left texture!");
            return false;
        }
    }

    // The position corresponds to the center of the texture.
    float wr = raider_right_texture.width * 0.5f;
    float hr = raider_right_texture.height * 0.5f;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, vertices, GL_STATIC_DRAW);

    // Index Buffer creation
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

    // Vertex Array (Container for Vertex + Index buffer)
    glGenVertexArrays(1, &mesh.vao);
    if (gl_has_errors())
        return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("raider.vs.glsl"), shader_path("raider.fs.glsl")))
        return false;

    // Setting initial values
    motion.position = { 300.f, 300.f };
    motion.radians = 0.f;
    motion.speed = 200.f;

    physics.scale = { 0.30f / 4, 0.28f };

    m_is_alive = true;
    m_is_move = false;
    m_light_up_countdown_ms = -1.f;


    return true;
}

// Releases all graphics resources
void Raider::destroy() {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

// Called on each frame by World::update()
void Raider::update(float ms, vec2 screen) {
    if (m_is_alive) {
        // check left and right bound
        if (motion.position.x < 50.f) {
            move({ 1.0f ,y_velocity });
        }
        else if (motion.position.x > screen.x - 50.f) {
            move({ -1.0f ,y_velocity });
        }
        else {
            move({ x_velocity, y_velocity });
        }

    }
    else {
        // If dead
        move({ 0.f, y_velocity });

        // if we are facing right, rotate right
        if (s_direction == 1)
        {
            set_rotation(motion.radians += rotation_speed);
        }

        // if we are facing left, rotate left
        else
        {
            set_rotation(motion.radians -= rotation_speed);
        }
    }
}

void Raider::draw(const mat3& projection) {
    transform.begin();
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    transform.end();

    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_DEPTH_TEST);

    // Getting uniform locations
    GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
    GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
    GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
    GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");
    GLint numFrames_uloc = glGetUniformLocation(effect.program, "numFrames");
    GLint currentFrame_uloc = glGetUniformLocation(effect.program, "currentFrame");

    // Setting vertices and indices
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

    // Input data location as in the vertex buffer
    //todo differences here
    GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(effect.program, "in_texcoord");
    glEnableVertexAttribArray(in_position_loc);
    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
    glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*) sizeof(vec3));

    //adding part
    // Enabling and binding texture to slot 0
    glActiveTexture(GL_TEXTURE0);
    if (s_direction == 1) {
        glBindTexture(GL_TEXTURE_2D, raider_right_texture.id);
    }
    else if (s_direction == 2) {
        glBindTexture(GL_TEXTURE_2D, raider_left_texture.id);
    }

    // Setting uniform values to the currently bound program
    // currFrame offset starts at 0!!!!!!
    int numFrame = 4;
    int currFrame;
    if (m_is_move && m_is_alive && m_is_onPlatform) {
        if (walking_loop >= 400) {
            currFrame = 1;
            walking_loop -= 20;
            walk_current_frame = 0;
        }
        else if (walking_loop >= 300) {
            currFrame = 0;
            walking_loop -= 20;
            walk_current_frame = 1;
        }
        else if (walking_loop >= 200) {
            currFrame = 2;
            walking_loop -= 20;
            walk_current_frame = 2;
        }
        else if (walking_loop >= 100) {
            currFrame = 0;
            walking_loop -= 20;
            walk_current_frame = 3;
        }
        else {
            currFrame = 1;
            walking_loop = 500;
            walk_current_frame = 0;
        }
    }
    else {
        currFrame = 1;
        walk_current_frame = 0;
    }
    if (m_jump && m_is_alive) {
        currFrame = 2;
    }


    glUniform1iv(numFrames_uloc, 1, &numFrame);
    glUniform1iv(currentFrame_uloc, 1, &currFrame);

    //todo differences here
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)& transform.out);
    //  Raider Color
    float color[] = { 1.f, 1.f, 1.f };
    if (!this->m_is_alive) {
        color[1] = 0.f;
        color[2] = 0.f;

    }
    // turn red when standing on lava
    if (time_spent_on_lava>0) {
        color[1] = 1.f - time_spent_on_lava/1200;
        color[2] = 1.f - time_spent_on_lava/1200;
    }

    glUniform3fv(color_uloc, 1, color);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)& projection);

    //fixme::!!!!!!!!!!!!!!!!!!! leave it for future function
//    int light_up = 0;
//    if (this->m_light_up_countdown_ms > 0) {
//        light_up = 1;
//    }
//    glUniform1iv(light_up_uloc, 1, &light_up);

    // Drawing!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}


bool Raider::collides_with(const Platform& platform) {
    //collision x-axis
    bool collisionX =
        motion.position.x + 0.5 * this->get_bounding_box().x >= platform.get_position().x - 0.5 * platform.get_bounding_box().x &&
        platform.get_position().x + 0.5 * platform.get_bounding_box().x >= motion.position.x - 0.5 * this->get_bounding_box().x;
    // Collision y-axis?
    bool collisionY =
        motion.position.y + 0.5 * this->get_bounding_box().y >= platform.get_position().y - 0.5 * platform.get_bounding_box().y &&
        platform.get_position().y + 0.5 * platform.get_bounding_box().y >= motion.position.y + 0.5 * this->get_bounding_box().y;
    //collision only on both
    return collisionX && collisionY;
}
bool Raider::above(const Platform& platform) {
    return motion.position.y < platform.get_position().y;
}

bool Raider::on_top_of(const Platform& platform)
{
    if (platform.type == 3 && platform.alpha < 0.30f)
    {
        return false;
    }

    return above(platform) && collides_with(platform);
}

bool Raider::collides_with(const Spike& spike)
{
    float dx = motion.position.x - spike.get_position().x;
    float dy = motion.position.y - spike.get_position().y;
    float d_sq = dx * dx + dy * dy;
    float other_r = std::max(spike.get_bounding_box().x, spike.get_bounding_box().y);
    float my_r = std::max(physics.scale.x, physics.scale.y);
    float r = std::max(other_r, my_r);
    r *= 1.42f;
    if (d_sq < r * r)
        return true;
    return false;
}

bool Raider::collides_with(const fallingSpike& spike) {
    float dx = motion.position.x - spike.get_position().x;
    float dy = motion.position.y - spike.get_position().y;
    float d_sq = dx * dx + dy * dy;
    // float other_r = std::max(spike.get_bounding_box().x, spike.get_bounding_box().y);
    // float my_r = std::max(physics.scale.x, physics.scale.y);
    // float r = std::max(other_r, my_r);
    // r *= 1.42f;
    if (d_sq < 1250.0f)
        return true;
    return false;
}
bool Raider::collides_with(const Fireball& spike) {
    float dx = motion.position.x - spike.get_position().x;
    float dy = motion.position.y - spike.get_position().y;
    float d_sq = dx * dx + dy * dy;
    // float other_r = std::max(spike.get_bounding_box().x, spike.get_bounding_box().y);
    // float my_r = std::max(physics.scale.x, physics.scale.y);
    // float r = std::max(other_r, my_r);
    // r *= 1.42f;
    if (d_sq < 1000.0f)
        return true;
    return false;
}

bool Raider::collides_with(const Coin& coin) {
    bool collisionX =
            motion.position.x + 0.5 * this->get_bounding_box().x >= coin.get_position().x - 0.5 * coin.get_bounding_box().x &&
            coin.get_position().x + 0.5 * coin.get_bounding_box().x >= motion.position.x - 0.5 * this->get_bounding_box().x;
    // Collision y-axis?
    bool collisionY =
            motion.position.y + 0.5 * this->get_bounding_box().y >= coin.get_position().y - 0.5 * coin.get_bounding_box().y &&
            coin.get_position().y + 0.5 * coin.get_bounding_box().y >= motion.position.y - 0.5 * this->get_bounding_box().y;
    //collision only on both
    return collisionX && collisionY;
}

bool Raider::collides_with(const Bouncingfireball &b_fireball) {
    bool collisionX =
            motion.position.x + 0.5 * this->get_bounding_box().x >= b_fireball.get_position().x - 0.5 * b_fireball.get_bounding_box().x &&
            b_fireball.get_position().x + 0.5 * b_fireball.get_bounding_box().x >= motion.position.x - 0.5 * this->get_bounding_box().x;
    // Collision y-axis?
    bool collisionY =
            motion.position.y + 0.5 * this->get_bounding_box().y >= b_fireball.get_position().y - 0.5 * b_fireball.get_bounding_box().y &&
            b_fireball.get_position().y + 0.5 * b_fireball.get_bounding_box().y >= motion.position.y - 0.5 * this->get_bounding_box().y;
    //collision only on both
    return collisionX && collisionY;
}


vec2 Raider::get_position() const {
    return motion.position;
}

void Raider::move(vec2 off) {
    motion.position.x += off.x;
    motion.position.y += off.y;
}

void Raider::set_rotation(float radians) {
    motion.radians = radians;
}

bool Raider::is_alive() const {
    return m_is_alive;
}
//fixme::need to be changed
vec2 Raider::get_bounding_box() const {
    //hat_width:naive solution for bounding box, hat is wider than body!!!
    int hat_width = 30;
    return { std::fabs(physics.scale.x) * raider_right_texture.width - hat_width, std::fabs(physics.scale.y) * raider_right_texture.height };
}

// Called when the Raider collides with a platform
void Raider::kill() {
    m_is_alive = false;
}
