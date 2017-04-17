// post_processing.h - Header file containing functions related
// to post-processing techniques
// Last modified - 17/04/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

void load_post_processing(array<frame_buffer, 2> &temp_frames, frame_buffer &first_pass, array<frame_buffer, 2> &frames, frame_buffer &temp_frame, geometry &screen_quad, texture &alpha_map, map<string, effect> &effects) {
	// Create 2 frame buffers - use screen width and height
	temp_frames[0] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	temp_frames[1] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create a first_pass frame
	first_pass = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());

	// Create frame buffer - use screen width and height
	frames[0] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	frames[1] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create a temporary frame buffer
	temp_frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create screen quad
	vector<vec3> screen_positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
	vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> screen_tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(screen_positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(screen_tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);

	// Load in texture for masking
	alpha_map = texture("textures/cockpit.jpg");

	// SHADERS
	// Motion blur effect
	effects["motion_blur"].add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	effects["motion_blur"].add_shader("shaders/motion_blur.frag", GL_FRAGMENT_SHADER);
	effects["motion_blur"].build();

	effects["tex_eff"].add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	effects["tex_eff"].add_shader("shaders/screen.frag", GL_FRAGMENT_SHADER);
	effects["tex_eff"].build();

	// Cockpit effect
	effects["cockpit_eff"].add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	effects["cockpit_eff"].add_shader("shaders/mask.frag", GL_FRAGMENT_SHADER);
	effects["cockpit_eff"].build();

	// Blur effect
	effects["blur"].add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	effects["blur"].add_shader("shaders/blur.frag", GL_FRAGMENT_SHADER);
	effects["blur"].build();

	// Depth of field effect
	effects["dof"].add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	effects["dof"].add_shader("shaders/depth_of_field.frag", GL_FRAGMENT_SHADER);
	effects["dof"].build();
}