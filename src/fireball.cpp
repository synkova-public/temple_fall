#include "fireball.hpp"
#include <iostream>
#include <cmath>
vec2 Fireball::get_bounding_box() {
    return { 30, 30 };
}

void Fireball::draw(const mat3& projection) {
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

    GLint size = 0;
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    // Drawing!
    glDrawArrays(GL_TRIANGLE_FAN, 0, 7);
}
bool Fireball::init() {
    m_vertices.clear();
    // Clearing errors
    gl_flush_errors();
    float radius = 4;
    int number_of_vertices = 7;
    for (float i = 0; i < 2 * 3.14; i += 2 * 3.14 / number_of_vertices) {
        vec3 vertex = { (float)cos(i) * radius, (float)sin(i) * radius, 0 };
        vec3 color = { 0.f, 0.f, 1.f };
        m_vertices.push_back({ vertex,color });
    }



    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

    // Vertex Array (Container for Vertex + Index buffer)
    glGenVertexArrays(1, &mesh.vao);
    if (gl_has_errors())
        return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("salmon.vs.glsl"), shader_path("salmon.fs.glsl")))
        return false;

    // Setting initial values
    motion.position = { -10.f, 70.f };
    motion.radians = 0.f;

    physics.scale = { 3, 3 };

    // m_light_up_countdown_ms = -1.f;

    return true;
}
void Fireball::update(float ms, vec2 seek) {
    motion.position.x += rand() % 7 - 3 + seek.x / 200;
    motion.position.y += rand() % 7 - 3 + seek.y / 200;
}

void Fireball::set_position(vec2 pos) {
    motion.position = pos;
}

vec2 Fireball::get_position() const {
    return motion.position;
}
void Fireball::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

// void parabolicFireball::reset(float xvel, float height) {
//   motion.position.y = height;
//   motion.position.x = ( xvel > 0 ) 0 : 600;
//   motion.speed = xvel;
// }

bool parabolicFireball::init() {
    m_vertices.clear();
    // Clearing errors
    vertical_speed = 0;
    cast(80, 0.2);
    float radius = 4;
    int number_of_vertices = 7;
    for (float i = 0; i < 2 * 3.14; i += 2 * 3.14 / number_of_vertices) {
        vec3 vertex = { (float)cos(i) * radius, (float)sin(i) * radius, 0 };
        vec3 color = { 1.f, 0.f, 0.f };
        m_vertices.push_back({ vertex,color });
    }



    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

    // Vertex Array (Container for Vertex + Index buffer)
    glGenVertexArrays(1, &mesh.vao);
    if (gl_has_errors())
        return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("salmon.vs.glsl"), shader_path("salmon.fs.glsl")))
        return false;

    // Setting initial values
    motion.position = { -120.f, 20.f };
    motion.radians = 0.f;

    physics.scale = { 5, 5 };

    // m_light_up_countdown_ms = -1.f;

    return true;
}
void parabolicFireball::update(float ms) {
    if (motion.position.y < 800) {
        motion.position.x += ms * motion.speed;
        motion.position.y += ms * vertical_speed;
        vertical_speed += ms * 9.8 / 50000;
    }
    else {
        cast(20, 0.2);
    }
}

void parabolicFireball::cast(float height, float speed) {
    motion.speed = speed;
    motion.position.x = (speed > 0) ? -120 : 600;
    motion.position.y = height;
    vertical_speed = 0;
}
