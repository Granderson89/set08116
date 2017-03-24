#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

void chase_camera_update(chase_camera &ccam, float delta_time, mesh target_mesh, mesh &stars, double &cursor_x, double &cursor_y)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// The ratio of pixels to rotation
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_height());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate camera by delta
	ccam.rotate(vec3(delta_y, delta_x, 0.0f));
	// Move camera - update target position and rotation
	ccam.move(target_mesh.get_transform().position, vec3(0.0f, 0.0f, 0.0f));
	// Update the camera
	ccam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;
	// Set skybox position to camera position (camera in centre of skybox)
	stars.get_transform().position = ccam.get_position();
}

void free_camera_update(free_camera &fcam, float delta_time, mesh &stars, double &cursor_x, double &cursor_y)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// The ratio of pixels to rotation
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_height());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate cameras by delta
	fcam.rotate(delta_x, delta_y);
	// Use keyboard to move the camera - WSAD
	if (glfwGetKey(renderer::get_window(), 'W')) {
		fcam.move(vec3(0.0f, 0.0f, 1.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		fcam.move(vec3(0.0f, 0.0f, -1.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		fcam.move(vec3(-1.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		fcam.move(vec3(1.0f, 0.0f, 0.0f));
	}
	// Update the camera
	fcam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;
	// Set skybox position to camera position (camera in centre of skybox)
	stars.get_transform().position = fcam.get_position();
}

void target_camera_update(target_camera &tcam, float delta_time, mesh &stars)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	// Use keyboard to change camera location
	// 1 - (50, 10, 50)
	if (glfwGetKey(renderer::get_window(), '1')) {
		tcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	}
	// 2 - (-50, 10, 50)
	if (glfwGetKey(renderer::get_window(), '2')) {
		tcam.set_position(vec3(-50.0f, 10.0f, 50.0f));
	}
	// 3 - (-50, 10, -50)
	if (glfwGetKey(renderer::get_window(), '3')) {
		tcam.set_position(vec3(-50.0f, 10.0f, -50.0f));
	}
	// 4 - (50, 10, -50)
	if (glfwGetKey(renderer::get_window(), '4')) {
		tcam.set_position(vec3(50.0f, 10.0f, -50.0f));
	}
	// Update the camera
	tcam.update(delta_time);
	// Set skybox position to camera position (camera in centre of skybox)
	stars.get_transform().position = tcam.get_position();

}

void load_cameras(target_camera &tcam, free_camera &fcam, chase_camera &ccam)
{
	// Set target camera properties
	tcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	tcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	tcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	// Set free camera properties
	fcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	fcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	fcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	// Set chase camera properties
	ccam.set_pos_offset(vec3(0.0f, 2.0f, 10.0f));
	ccam.set_springiness(0.5f);
	ccam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

}

void update_active_camera(bool chase_camera_active, bool free_camera_active,
	chase_camera &ccam, free_camera &fcam, target_camera &tcam, mesh &stars, mesh target_mesh, float delta_time, double &cursor_x, double &cursor_y)
{
	if (chase_camera_active)
	{
		chase_camera_update(ccam, delta_time, target_mesh, stars, cursor_x, cursor_y);
	}
	else if (free_camera_active)
	{
		free_camera_update(fcam, delta_time, stars, cursor_x, cursor_y);
	}
	else
	{
		target_camera_update(tcam, delta_time, stars);
	}
}