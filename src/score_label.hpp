#pragma once

#include "common.hpp"

// Salmon food
class Score_Label : public Entity
{
	// Shared between all score_label, no need to load one for each instance
	static Texture score_label_texture;

    static Texture high_score_1_show_texture;
    static Texture high_score_2_show_texture;
    static Texture high_score_3_show_texture;
    static Texture high_score_4_show_texture;
    static Texture high_score_5_show_texture;

    static Texture rank1_texture;
    static Texture rank2_texture;
    static Texture rank3_texture;
    static Texture rank4_texture;
    static Texture rank5_texture;
    static Texture no_rank_texture;

public:
    bool high_score_show;
    int  high_score_numb;
    bool death_screen_rank_show;
    int  death_screen_rank;

	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();

	// Update score_label
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the score_label
	// projection is the 2D orthographic projection matrix
	void draw(const mat3& projection) override;

	// Returns the current score_label position
	vec2 get_position() const;

	// Sets the new score_label position
	void set_position(vec2 position);

	// Returns the score_label' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

    void set_scale(vec2 scale);

	float width_scaled;
	float width_unscaled;

	float height_scaled;
	float height_unscaled;
};
