#pragma once

#include "common.hpp"
#include <vector>

class Platform;

class Spike;

class fallingSpike;

class Fireball;

class Coin;

class Bouncingfireball;

class Raider : public Entity {
    static Texture raider_right_texture;
    static Texture raider_left_texture;
public:
    float x_velocity, y_velocity, speed, jump_speed, rotation_speed;
    bool candown, canup, canright, canleft;
    // character's direction; 1 for up, 2 for down, 3 for left, 4 for right
    int s_direction;
    bool on_platform;
    bool m_is_move; //true if the raider is moving
    bool m_jump; //ture if the raider is jumping
    bool m_is_onPlatform;
    float walking_loop;
    float time_spent_on_lava;

    int walk_current_frame;
    int coin_collected_so_far;
    int coin_current;
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all associated resources
    void destroy();

    // Update raider position based on direction
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms, vec2 screen);

    // Renders the raider
    void draw(const mat3& projection) override;

    // Collision routines for turtles and fish
    bool collides_with(const Platform& platform);

    bool on_top_of(const Platform& platform);

    bool above(const Platform& platform);

    bool hitting_the_side(const Platform& platform);

    bool collides_with(const Spike& spike);

    bool collides_with(const fallingSpike& fallingSpike);

    bool collides_with(const Fireball& fireball);

    bool collides_with(const Coin& coin);

    bool collides_with(const Bouncingfireball& b_fireball);
    // Returns the current raider position
    vec2 get_position() const;

    // Returns the bounding box of raider
    vec2 get_bounding_box() const;

    // Moves the raider's position by the specified offset
    void move(vec2 off);

    // Set raider rotation in radians
    void set_rotation(float radians);

    // True if the raider is alive
    bool is_alive() const;

    // Kills the raider, changing its alive state and triggering on death events
    void kill();

    // Called when the raider collides with a fish, starts lighting up the raider
    void light_up();

private:
    float m_light_up_countdown_ms; // Used to keep track for how long the raider should be lit up
    bool m_is_alive; // True if the raider is alive

    std::vector <Vertex> m_vertices;
    std::vector <uint16_t> m_indices;
};
