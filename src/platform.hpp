#pragma once

#include "common.hpp"


// Salmon food
class Platform : public Entity
{
    // Shared between all fish, no need to load one for each instance
    static Texture platform_texture;
    static Texture tar_texture;
    static Texture lava_texture;
    static Texture ghost_texture;


public:
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all the associated resources
    void destroy();

    // Update fish
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms);

    // Renders the fish
    // projection is the 2D orthographic projection matrix
    void draw(const mat3& projection) override;

    // Returns the current fish position
    vec2 get_position() const;

    // Sets the new fish position
    void set_position(vec2 position);

    // Returns the fish' bounding box for collision detection, called by collides_with()
    vec2 get_bounding_box() const;

    int type; // 0 is regular platform, 1 is tar platform, 2 is lava platform
    float alpha;
    vec2 lightsource;
    float illumination;
    bool used;
};
