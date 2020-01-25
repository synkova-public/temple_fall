#pragma once

#include "common.hpp"

// Salmon food
class Coin : public Entity
{
	// Shared between all coins, no need to load one for each instance
	static Texture coin_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();
	
	// Update coin
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the coin
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection) override;

	// Returns the current coin position
	vec2 get_position() const;

	// Sets the new coin position
	void set_position(vec2 position);

	// Returns the coin' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	//set the speed of coin
	void set_speed(float speed);

  float illumination;
};
