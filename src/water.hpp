#pragma once

#include "common.hpp"


class Water : public Entity
{
public:
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all associated resources
    void destroy();

    // Renders the water
    void draw(const mat3& projection)override;

    // raider dead time getters and setters
    void set_raider_dead();
    void reset_raider_dead_time();
    float get_raider_dead_time() const;

private:
    // When raider is alive, the time is set to -1
    float m_dead_time;
};
