#include "fallingSpike.hpp"
#include <iostream>
#include <cmath>

vec2 fallingSpike::get_bounding_box() {
    return { 30, 30 };
}

void fallingSpike::draw(const mat3& projection)
{
    transform.begin();
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    transform.end();

    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // Getting uniform locations
    GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
    GLint color_uloc = glGetUniformLocation(effect.program, "fcolor");
    GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
    GLint light_up_uloc = glGetUniformLocation(effect.program, "light_up");

    // Setting vertices and indices
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

    // Input data location as in the vertex buffer
    GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
    GLint in_color_loc = glGetAttribLocation(effect.program, "in_color");
    glEnableVertexAttribArray(in_position_loc);
    glEnableVertexAttribArray(in_color_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));

    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)& transform.out);

    // !!! Salmon Color
    float color[] = { 1.f, 1.f, 1.f };
    glUniform3fv(color_uloc, 1, color);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)& projection);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // HERE TO SET THE CORRECTLY LIGHT UP THE SALMON IF HE HAS EATEN RECENTLY
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // int light_up = ( m_light_up_countdown_ms > 1) ? 1 : 0;
  // printf("m_light_up_countdown_ms: %f\n", m_light_up_countdown_ms);
    // glUniform1iv(light_up_uloc, 1, &light_up);

    // Get number of infices from buffer,
    // we know our vbo contains both colour and position information, so...
    GLint size = 0;
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GLsizei num_indices = size / sizeof(uint16_t);

    GLint illumination_uloc = glGetUniformLocation(effect.program, "illumination");
    glUniform1f(illumination_uloc, illumination);
    // Drawing!
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
}

bool fallingSpike::init()
{
    played_falling_sound = false;

    m_vertices.clear();
    m_indices.clear();

    // Clearing errors
    gl_flush_errors();

    vec3 black = {0.0f, 0.0f, 0.0f};
    vec3 white = {1.0f, 1.0f, 1.0f};
    vec3 grey  = {0.5f, 0.5f, 0.5f};

    Vertex a = {{-15.0f, -25.0f, 00.0f}, grey};
    Vertex b = {{+00.0f, +25.0f, 00.0f}, grey};
    Vertex c = {{+15.0f, -25.0f, 00.0f}, white};

    m_vertices.push_back(a);
    m_vertices.push_back(b);
    m_vertices.push_back(c);
    m_indices.push_back(0);
    m_indices.push_back(1);
    m_indices.push_back(2);

    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

    // Index Buffer creation
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

    // Vertex Array (Container for Vertex + Index buffer)
    glGenVertexArrays(1, &mesh.vao);
    if (gl_has_errors())
        return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("illuminated_vertices.vs.glsl"), shader_path("illuminated_vertices.fs.glsl")))
        return false;
    // Setting initial values
    motion.position = { 40.f, -50.f };
    motion.radians = 0.f;
    motion.speed = 1.f;
    physics.scale = { 1, 1 };
    isFalling = false;

    // m_light_up_countdown_ms = -1.f;

    return true;
}

// Releases all graphics resources
void fallingSpike::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

vec2 fallingSpike::get_position() const {
    return motion.position;
}

void fallingSpike::set_position(vec2 position) {
    motion.position = position;
}

// Called on each frame by World::update()
void fallingSpike::update(float ms)
{
    if (isFalling && motion.position.y < 0.0f)
    {
        motion.speed = 450.0f;

        float step = motion.speed * (ms / 1000);

        motion.position.y += step;
    }

    else if (isFalling && motion.position.y >= 0.0f)
    {
        motion.speed += ms * 10.0f / 70;

        float step = motion.speed * (ms / 1000);

        motion.position.y += step;
    }
}

void fallingSpike::cast() {
    motion.speed = 0;
}

void fallingSpike::set_speed(float speed) {
    motion.speed = speed;
}

void fallingSpike::set_isFalling(bool cond) {
    isFalling = cond;
}
bool fallingSpike::get_isFalling() {
    return isFalling;
}

bool fallingSpike::collides_exact_with(const Raider &raider, const mat3& projection)
{
    // currently creating matrices
    transform.begin();

    transform.translate({motion.position.x, motion.position.y});
    transform.rotate(motion.radians);
    transform.scale(physics.scale);

    transform.end();

    // currently multiplying matrices
    Vertex c_vertex_a   = m_vertices[0];
    Vertex c_vertex_b   = m_vertices[1];
    Vertex c_vertex_c   = m_vertices[2];

    vec3 c_position_a   = c_vertex_a.position;
    vec3 c_position_b   = c_vertex_b.position;
    vec3 c_position_c   = c_vertex_c.position;

    vec3 multi_before_a = vec3{c_position_a.x, c_position_a.y, 1.0f};
    vec3 multi_before_b = vec3{c_position_b.x, c_position_b.y, 1.0f};
    vec3 multi_before_c = vec3{c_position_c.x, c_position_c.y, 1.0f};

    vec3 multi_after_a  = mul(mul(projection, transform.out), multi_before_a);
    vec3 multi_after_b  = mul(mul(projection, transform.out), multi_before_b);
    vec3 multi_after_c  = mul(mul(projection, transform.out), multi_before_c);

    vec3 actual_a       = vec3{multi_after_a.x * 300.0f + 300.0f, multi_after_a.y * -400.0f + 400.0f, 1.0f};
    vec3 actual_b       = vec3{multi_after_b.x * 300.0f + 300.0f, multi_after_b.y * -400.0f + 400.0f, 1.0f};
    vec3 actual_c       = vec3{multi_after_c.x * 300.0f + 300.0f, multi_after_c.y * -400.0f + 400.0f, 1.0f};

    // generating raider variables
    vec2  raider_pos   = raider.get_position();
    vec2  raider_dim   = raider.get_bounding_box();

    float raider_max_x = raider_pos.x + raider_dim.x/2.0f;
    float raider_min_x = raider_pos.x - raider_dim.x/2.0f;

    float raider_max_y = raider_pos.y + (raider_dim.y + 10.0f)/2.0f;
    float raider_min_y = raider_pos.y - (raider_dim.y + 10.0f)/2.0f;

    // comparing the normal points
    if (raider_min_x <= actual_a.x && actual_a.x <= raider_max_x &&
        raider_min_y <= actual_a.y && actual_a.y <= raider_max_y)
    {
        return true;
    }

    if (raider_min_x <= actual_b.x && actual_b.x <= raider_max_x &&
        raider_min_y <= actual_b.y && actual_b.y <= raider_max_y)
    {
        return true;
    }

    if (raider_min_x <= actual_c.x && actual_c.x <= raider_max_x &&
        raider_min_y <= actual_c.y && actual_c.y <= raider_max_y)
    {
        return true;
    }

    // comparing the middle points
    float mid_1_x = (actual_a.x + actual_b.x)/2.0f;
    float mid_1_y = (actual_a.y + actual_b.y)/2.0f;

    float mid_2_x = (actual_a.x + actual_c.x)/2.0f;
    float mid_2_y = (actual_a.y + actual_c.y)/2.0f;

    float mid_3_x = (actual_c.x + actual_b.x)/2.0f;
    float mid_3_y = (actual_c.y + actual_b.y)/2.0f;

    float mid_4_x = (actual_a.x + actual_b.x + actual_c.x)/2.0f;
    float mid_4_y = (actual_a.y + actual_b.y + actual_c.y)/2.0f;

    if (raider_min_x <= mid_1_x && mid_1_x <= raider_max_x &&
        raider_min_y <= mid_1_y && mid_1_y <= raider_max_y)
    {
        return true;
    }

    if (raider_min_x <= mid_2_x && mid_2_x <= raider_max_x &&
        raider_min_y <= mid_2_y && mid_2_y <= raider_max_y)
    {
        return true;
    }

    if (raider_min_x <= mid_3_x && mid_3_x <= raider_max_x &&
        raider_min_y <= mid_3_y && mid_3_y <= raider_max_y)
    {
        return true;
    }

    if (raider_min_x <= mid_4_x && mid_4_x <= raider_max_x &&
        raider_min_y <= mid_4_y && mid_4_y <= raider_max_y)
    {
        return true;
    }

    return false;
}

bool fallingSpike::collides_exact_with(const Raider& raider) {
    transform.begin();
    transform.translate({ motion.position.x, motion.position.y });
    transform.scale(physics.scale);
    transform.end();
    vec2 rp = raider.get_position();
    vec2 rb = raider.get_bounding_box();
    float min_x = rp.x;
    float max_x = rp.x + rb.x;
    float min_y = rp.y;
    float max_y = rp.y + rb.y;
    for (int i = 0; i < m_vertices.size(); i++) {
        vec3 affine = m_vertices[i].position; affine.z = 1.0f;
        vec3 trans = mul(transform.out, affine);
        if (min_x < trans.x < max_x && min_y < trans.y < min_y) {
            return true;
        }
    }
    return false;
}
