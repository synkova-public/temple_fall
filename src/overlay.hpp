#pragma once

#include "common.hpp"

// Salmon food
class gOverlay : public Entity
{
	// Shared between all overlay, no need to load one for each instance
	Texture overlay_texture_on;
	Texture overlay_texture_off;

public:
	bool displayed_on;

	// Creates all the associated render resources and default transform
	bool init(std::string on_path, std::string off_path);

	// Releases all the associated resources
	void destroy();
	
	// Update overlay
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the overlay
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection) override;

	// Returns the current overlay position
	vec2 get_position() const;

	// Sets the new overlay position
	void set_position(vec2 position);

	// Returns the overlay' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	float width_scaled;
	float width_unscaled;

	float height_scaled;
	float height_unscaled;
};
