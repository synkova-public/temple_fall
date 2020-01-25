#pragma once

#include "common.hpp"
#include <vector>

class Warning : public Entity
{
    static Texture warning_texture;

public:
    // Creates all the associated render resources and default transform
    bool init();
    // Releases all associated resources
    void destroy();

    // Renders the ceil
    void draw(const mat3& projection)override;
    void update(float ms, vec2 pos);

    // Sets the new spike position
    void set_position(vec2 position);

    // Width getter
    int get_width();

    // Returns the current spike position
    vec2 get_position() const;

    // Returns the spike' bounding box for collision detection, called by collides_with()
    vec2 get_bounding_box() const;

};

