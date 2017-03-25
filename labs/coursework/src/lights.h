// lights.h - Header file containing light functions
// Function to load the lights
// Last modified - 24/03/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Load the lights
void load_lights(vector<point_light> &points, vector<spot_light> &spots, vector<point_light> &points_rama, vector<spot_light> &spots_rama, vec3 rama_pos)
{
	// Set point light values, Position
	points[0].move(vec3(0.0f, 0.0f, 0.0f));
	// Light colour white
	points[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// Set range to 20
	points[0].set_range(1000.0f);

	// Set spot light values
	// Yellow light
	spots[0].set_position(vec3(90.0f, 40.0f, 60.0f));
	spots[0].set_light_colour(vec4(1.0f, 1.0f, 0.0f, 1.0f));
	spots[0].set_direction(normalize(vec3(-1.0f, -1.0f, 0.0f)));
	spots[0].set_range(100.0f);
	spots[0].set_power(0.1f);

	// Set Rama lights
	points_rama[0].set_position(rama_pos);
	points_rama[0].set_light_colour(vec4(1.0f));
	points_rama[0].set_range(100.0f);

	spots_rama[0].set_position(rama_pos + vec3(5.0f, 0.0f, 0.0f));
	spots_rama[0].set_light_colour(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	spots_rama[0].set_direction(normalize(vec3(-1.0f, 0.0f, 0.0f)));
	spots_rama[0].set_range(70.0f);
	spots_rama[0].set_power(0.1f);

	spots_rama[1].set_position(rama_pos - vec3(5.0f, 0.0f, 0.0f));
	spots_rama[1].set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	spots_rama[1].set_direction(normalize(vec3(1.0f, 0.0f, 0.0f)));
	spots_rama[1].set_range(70.0f);
	spots_rama[1].set_power(0.1f);
}