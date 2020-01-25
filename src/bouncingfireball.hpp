#pragma once

#include "common.hpp"
#include <vector>

class Platform;

// Salmon food
class Bouncingfireball : public Entity
{
	// Shared between all coins, no need to load one for each instance
	static Texture b_fireball_texture;

public:
//	struct Bouncingfireball {
////		float life = 0.0f; // remove pebble when its life reaches 0
//		vec2 position;
//		vec2 velocity;
//	};
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();
	
	// Update bouncingFireball
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the bouncingFireball
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection) override;

	// Returns the current bouncingFireball position
	vec2 get_position() const;

	// Sets the new bouncingFireball position
	void set_position(vec2 position);

	// Returns the cobouncingFireballin' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	//set the speed of bouncingFireball
	void set_speed(float speed);

	//set the vertical
	void set_vertical_speed(float speed);

	//get a approximate radius
	float get_approximate_radius();

	//check bouncingfireball platform collision and bounce
	void collides_with(std::vector<Platform> m_platform);

	bool collides_with(Platform platform);

	void set_velocity(vec2 v);

	vec2 get_velocity();

	bool between_Platform(Platform platform);

	bool between_side_Platform(Platform platform);

	void move(vec2 off);

private:
	float vertical_speed;
	vec2 velocity;
};
