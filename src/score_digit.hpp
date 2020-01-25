#pragma once

#include "common.hpp"

// Salmon food
class Score_Digit : public Entity
{
	// Shared between all score_digit, no need to load one for each instance
	static Texture score_digit_texture_0;
	static Texture score_digit_texture_1;
	static Texture score_digit_texture_2;
	static Texture score_digit_texture_3;
	static Texture score_digit_texture_4;
	static Texture score_digit_texture_5;
	static Texture score_digit_texture_6;
	static Texture score_digit_texture_7;
	static Texture score_digit_texture_8;
	static Texture score_digit_texture_9;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();
	
	// Update score_digit
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the score_digit
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection) override;

	// Returns the current score_digit position
	vec2 get_position() const;

	// Sets the new score_digit position
	void set_position(vec2 position);

	// Returns the score_digit' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	int m_number;

	float width_scaled;
	float width_unscaled;

	float height_scaled;
	float height_unscaled;
};