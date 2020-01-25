#pragma once

#include "common.hpp"

// Salmon food
class gScreen : public Entity
{
    // Shared between all screen, no need to load one for each instance
    static Texture screen_texture_title_selected_start;
    static Texture screen_texture_title_selected_lore;
    static Texture screen_texture_title_selected_score;
    static Texture screen_texture_title_selected_exit;

    static Texture screen_texture_lore_selected_return;
    static Texture screen_texture_score_selected_return;

    static Texture screen_texture_death_retry;
    static Texture screen_texture_death_menu;

public:
    int displayed_screen;

    // Creates all the associated render resources and default transform
    bool init();

    // Releases all the associated resources
    void destroy();

    // Update screen
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms);

    // Renders the screen
    // projection is the 2D orthographic projection matrix
    void draw(const mat3& projection) override;

    // Returns the current screen position
    vec2 get_position() const;

    // Sets the new screen position
    void set_position(vec2 position);

    // Returns the screen' bounding box for collision detection, called by collides_with()
    vec2 get_bounding_box() const;

    float width_scaled;
    float width_unscaled;

    float height_scaled;
    float height_unscaled;
};
