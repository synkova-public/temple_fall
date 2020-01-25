#pragma once

#include "common.hpp"
#include <vector>
#include "raider.hpp"
class fallingSpike : public Entity {
public:
    bool isFalling;
    bool played_falling_sound;

    void update(float ms);
    void destroy();
    bool init();
    vec2 get_bounding_box();
    vec2 get_position() const;
    void set_position(vec2 position);
    void set_isFalling(bool cond);
    bool get_isFalling();
    void set_speed(float speed);

    void draw(const mat3& projection)override;
    void cast();

    bool collides_exact_with(const Raider &raider, const mat3& projection_2D);
    bool collides_exact_with(const Raider& raider);
    float illumination;
private:
    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;
};
