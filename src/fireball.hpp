#pragma once

#include "common.hpp"
#include <vector>
class Fireball : public Entity {
public:
    void update(float ms, vec2 seek);
    void destroy();
    bool init();
    vec2 get_bounding_box();
    vec2 get_position() const;
    void set_position(vec2 pos);

    void draw(const mat3& projection)override;
private:
    std::vector<Vertex> m_vertices;
};

class parabolicFireball : public Fireball {
public:
    bool init();
    void cast(float height, float speed);
    void update(float ms);
private:
    float vertical_speed;
    std::vector<Vertex> m_vertices;
};
