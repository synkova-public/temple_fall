#pragma once
#include "common.hpp"

class Backdrop : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture backdrop_texture;
    static float fallen;
    static float elapsed;

public:
	bool init();
	void destroy();
	void update(float ms);
	void draw(const mat3& projection) override;
	vec2 get_position() const;
	void set_position(vec2 position);
	vec2 get_bounding_box() const;
  vec2 lightsource;
};
