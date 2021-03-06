// render_helpers.h - Header file containing render functions
// Functions to create a shadow map, render the different
// objects in the scene and render the fire particle effect
// render_fire not currently working properly
// Last modified - 20/04/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

// Types of fog
#define FOG_LINEAR 0
#define FOG_EXP 1
#define FOG_EXP2 2

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Create a shadow map from the pov of the spot light
void create_shadow_map(effect shadow_eff, 
					   map<string, mesh> solar_objects, array<mesh, 7> enterprise, array<mesh, 2> motions,  mesh rama, 
					   shadow_map &shadow, mat4 &LightProjectionMat)
{
	// Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);
	LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);
	// Bind shader
	renderer::bind(shadow_eff);
	// View matrix taken from shadow map
	auto V = shadow.get_view();
	// Render Enterprise
	for (size_t i = 0; i < enterprise.size(); i++) {
		// SET M to be the usual mesh transform matrix
		auto M = enterprise[i].get_transform().get_transform_matrix();
		// Apply the heirarchy chain
		for (size_t j = i; j > 0; j--) {
			M = enterprise[j - 1].get_transform().get_transform_matrix() * M;
		}
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"),
			1,
			GL_FALSE,
			value_ptr(MVP));
		// Render mesh
		renderer::render(enterprise[i]);
	}
	for (int i = 0; i < motions.size(); i++) {
		auto m = motions[i];
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"),
			1,
			GL_FALSE,
			value_ptr(MVP));
		// Render mesh
		renderer::render(m);
	}
	// Render solar_objects
	for (auto &e : solar_objects) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"),
			1,
			GL_FALSE,
			value_ptr(MVP));
		// Render mesh
		renderer::render(m);
	}
	// Render Rama
	auto m = rama;
	auto M = m.get_transform().get_transform_matrix();
	auto MVP = LightProjectionMat * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Render mesh
	renderer::render(m);
	// Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);
}

// Render the clouds around Earth
void render_clouds(effect cloud_eff, 
				   mesh clouds, 
				   texture cloudsTex, texture normal_map, 
				   vector<point_light> points, 
				   mat4 PV, vec3 cam_pos)
{
	// Render clouds
	renderer::bind(cloud_eff);
	// Create MVP matrix
	auto M = clouds.get_transform().get_transform_matrix();
	auto MVP = PV * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(cloud_eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(cloud_eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(cloud_eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(clouds.get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(clouds.get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind and set textures
	renderer::bind(cloudsTex, 0);
	glUniform1i(cloud_eff.get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(normal_map, 1);
	// Set normal_map uniform
	glUniform1i(cloud_eff.get_uniform_location("normal_map"), 1);
	// Set eye position- Get this from active camera
	glUniform3fv(cloud_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Render mesh
	renderer::render(clouds);
}

// Render the skybox
void render_skybox(effect skybox_eff, 
				   mesh stars, 
				   cubemap cube_map, 
				   mat4 PV)
{
	// Disable depth test, depth mask, face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(skybox_eff);
	// Calculate MVP for the skybox
	auto MVP = PV * stars.get_transform().get_transform_matrix();
	glUniformMatrix4fv(skybox_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Bind the cube map and set it
	renderer::bind(cube_map, 0);
	glUniform1i(skybox_eff.get_uniform_location("cubemap"), 0);
	// Render the skybox
	renderer::render(stars);
	// Enable depth test, depth mask, face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

// Render the basic planets etc.
void render_solar_objects(effect eff,
						  mesh m,
						  texture tex, texture normal_map,
						  shadow_map shadow, mat4 LightProjectionMat, 
						  mat4 PV)
{
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto MVP = PV * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind and set textures
	renderer::bind(tex, 0);
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Bind and set normal_map
	renderer::bind(normal_map, 1);
	glUniform1i(eff.get_uniform_location("normal_map"), 1);
	// Render mesh
	renderer::render(m);
}

// Render the sun
void render_sun(effect eff, 
				mesh m, 
				texture tex, texture normal_map, 
				vector<point_light> points, vector<spot_light> spots, 
				shadow_map shadow, mat4 LightProjectionMat, 
			    mat4 PV, vec3 cam_pos, 
				float explode_factor, float peak_factor, vec3 sun_activity)
{
	// Disable cull face
	glDisable(GL_CULL_FACE);
	// Bind sun effect
	renderer::bind(eff);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto MVP = PV * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind light
	renderer::bind(points, "points");
	// Bind spot light
	renderer::bind(spots, "spots");
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Set eye position
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind and set texture
	renderer::bind(tex, 0);
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Bind and set normal_map
	renderer::bind(normal_map, 1);
	glUniform1i(eff.get_uniform_location("normal_map"), 1);
	// Bind and set shadow map texture
	renderer::bind(shadow.buffer->get_depth(), 2);
	glUniform1i(eff.get_uniform_location("shadow_map"), 2);
	// Set explode factor uniform
	glUniform1f(eff.get_uniform_location("explode_factor"), explode_factor);
	// Set explode factor uniform
	glUniform1f(eff.get_uniform_location("peak_factor"), peak_factor);
	// Set explode factor uniform
	glUniform3fv(eff.get_uniform_location("sun_activity"), 1, value_ptr(sun_activity));
	// Render mesh
	renderer::render(m);
	glEnable(GL_CULL_FACE);
}

// Render Jupiter
void render_jupiter(effect eff,
					mesh m,
					array<texture, 14> texs,
					vector<point_light> points, vector<spot_light> spots,
					shadow_map shadow, mat4 LightProjectionMat,
					mat4 PV, vec3 cam_pos, 
					float weather_factor)
{
	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto MVP = PV * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind spot light
	renderer::bind(spots, "spots");
	// Decide which textures to bind
	int tex1, tex2;
	if (weather_factor < 0.07)
	{
		tex1 = 0;
		tex2 = 1;
	}
	else if (weather_factor < 0.14)
	{
		tex1 = 1;
		tex2 = 2;
	}
	else if (weather_factor < 0.21)
	{
		tex1 = 2;
		tex2 = 3;
	}
	else if (weather_factor < 0.28)
	{
		tex1 = 3;
		tex2 = 4;
	}
	else if (weather_factor < 0.35)
	{
		tex1 = 4;
		tex2 = 5;
	}
	else if (weather_factor < 0.42)
	{
		tex1 = 5;
		tex2 = 6;
	}
	else if (weather_factor < 0.49)
	{
		tex1 = 6;
		tex2 = 7;
	}
	else if (weather_factor < 0.56)
	{
		tex1 = 7;
		tex2 = 8;
	}
	else if (weather_factor < 0.63)
	{
		tex1 = 8;
		tex2 = 9;
	}
	else if (weather_factor < 0.70)
	{
		tex1 = 9;
		tex2 = 10;
	}
	else if (weather_factor < 0.77)
	{
		tex1 = 10;
		tex2 = 11;
	}
	else if (weather_factor < 0.84)
	{
		tex1 = 11;
		tex2 = 12;
	}
	else if (weather_factor < 0.91)
	{
		tex1 = 12;
		tex2 = 13;
	}
	else
	{
		tex1 = 13;
		tex2 = 0;
	}
	// Bind and set textures
	renderer::bind(texs[tex1], 0);
	renderer::bind(texs[tex2], 1);
	glUniform1i(eff.get_uniform_location("tex[0]"), 0);
	glUniform1i(eff.get_uniform_location("tex[1]"), 1);
	// Set eye position- Get this from active camera
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind and set shadow map texture
	renderer::bind(shadow.buffer->get_depth(), 2);
	glUniform1i(eff.get_uniform_location("shadow_map"), 2);
	// Render mesh
	renderer::render(m);
}

// Render the Enterprise (transform hierarchy)
void render_enterprise(effect ship_eff, 
					   array<mesh, 7> enterprise, array<mesh, 2> motions,
					   texture tex, texture normal_map, array<texture, 2> motions_textures,
					   vector<point_light> points, vector<spot_light> spots,
					   shadow_map shadow, mat4 &LightProjectionMat, 
					   mat4 PV, vec3 cam_pos)
{
	// Bind effect
	renderer::bind(ship_eff);
	// Set the texture value for the shader here
	glUniform1i(ship_eff.get_uniform_location("tex"), 0);
	// Bind material
	renderer::bind(enterprise[0].get_material(), "mat");
	// Bind point light
	renderer::bind(points, "points");
	// Bind spot light
	renderer::bind(spots, "spots");
	// Bind texture to renderer
	renderer::bind(tex, 0);
	// Bind normal_map
	renderer::bind(normal_map, 1);
	// Set normal_map uniform
	glUniform1i(ship_eff.get_uniform_location("normal_map"), 1);
	// Set eye position
	glUniform3fv(ship_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind shadow map texture
	renderer::bind(shadow.buffer->get_depth(), 2);
	// Set the shadow_map uniform
	glUniform1i(ship_eff.get_uniform_location("shadow_map"), 2);
	// Find the lcoation for the MVP uniform
	const auto loc = ship_eff.get_uniform_location("MVP");
	// Render Enterprise
	for (size_t i = 0; i < enterprise.size(); i++) {
		// SET M to be the usual mesh  transform matrix
		auto M = enterprise[i].get_transform().get_transform_matrix();
		// Apply the heirarchy chain
		for (size_t j = i; j > 0; j--) {
			M = enterprise[j - 1].get_transform().get_transform_matrix() * M;
		}
		// Set MVP matrix uniform
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
		// Set M matrix uniform
		glUniformMatrix4fv(ship_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(ship_eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(enterprise[i].get_transform().get_normal_matrix()));
		// Set lightMVP uniform
		auto lightMVP = LightProjectionMat * shadow.get_view() * M;
		glUniformMatrix4fv(ship_eff.get_uniform_location("lightMVP"),
			1,
			GL_FALSE,
			value_ptr(lightMVP));
		// Render mesh
		renderer::render(enterprise[i]);
	}
	// Bind material
	renderer::bind(motions[0].get_material(), "mat");
	// Bind texture
	renderer::bind(motions_textures[0], 0);
	for (int i = 0; i < motions.size(); i++)
	{
		auto M = motions[i].get_transform().get_transform_matrix();
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
		// Set M matrix uniform
		glUniformMatrix4fv(ship_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(ship_eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(motions[i].get_transform().get_normal_matrix()));
		// Set lightMVP uniform
		auto lightMVP = LightProjectionMat * shadow.get_view() * M;
		glUniformMatrix4fv(ship_eff.get_uniform_location("lightMVP"),
			1,
			GL_FALSE,
			value_ptr(lightMVP));
		renderer::render(motions[i]);
	}
}


// Render Rama
void render_rama(effect outside_eff, effect inside_eff, effect terrain_eff,
				 mesh rama, array<mesh, 6> rama_terrain,
				 texture inside, texture outside, texture grass, texture blend_map, texture normal_outside, texture normal_inside, material mat_inside,
				 array<texture, 4> terrain_texs,
				 vector<point_light> points, vector<spot_light> spots, vector<point_light> points_rama, vector<spot_light> spots_rama,
				 shadow_map shadow, mat4 &LightProjectionMat, 
				 mat4 PV, mat4 V, vec3 cam_pos)
{
	// Bind effect
	renderer::bind(outside_eff);
	// Create MVP matrix
	auto M = rama.get_transform().get_transform_matrix();
	auto MVP = PV * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(outside_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(outside_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(outside_eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(rama.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	glUniformMatrix4fv(outside_eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(rama.get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	// Bind textures
	renderer::bind(outside, 0);
	renderer::bind(grass, 1);
	// Bind blend map
	renderer::bind(blend_map, 2);
	// Set the uniform values for textures
	static int tex_indices[] = { 0, 1 };
	// Set tex uniform
	glUniform1iv(outside_eff.get_uniform_location("tex"), 2, tex_indices);
	// Set blend map uniform
	glUniform1i(outside_eff.get_uniform_location("blend_map"), 2);
	// Bind normal_map
	renderer::bind(normal_outside, 3);
	// Set normal_map uniform
	glUniform1i(outside_eff.get_uniform_location("normal_map"), 3);
	// Set eye position
	glUniform3fv(outside_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind shadow map texture
	renderer::bind(shadow.buffer->get_depth(), 4);
	// Set the shadow_map uniform
	glUniform1i(outside_eff.get_uniform_location("shadow_map"), 4);
	// Render mesh
	renderer::render(rama);
	
	// Disable face culling
	glDisable(GL_CULL_FACE);
	// Bind effect
	renderer::bind(inside_eff);
	// Create MV matrix
	auto MV = V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set MV matrix uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("MV"), 1, GL_FALSE, value_ptr(MV));
	// Set M matrix uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(inside_eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(rama.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(mat_inside, "mat");
	// Bind light
	renderer::bind(points_rama, "points");
	// Bind light
	renderer::bind(spots_rama, "spots");
	// Bind texture
	renderer::bind(inside, 0);
	// Set tex uniform
	glUniform1i(inside_eff.get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(normal_inside, 1);
	// Set normal_map uniform
	glUniform1i(inside_eff.get_uniform_location("normal_map"), 1);
	// Set fog colour
	glUniform4fv(inside_eff.get_uniform_location("fog_colour"), 1, value_ptr(vec4(0.412f, 1.0f, 0.996f, 1.0f)));
	// Set fog start:  5.0f
	glUniform1f(inside_eff.get_uniform_location("fog_start"), 5.0f);
	// Set fog end:  100.0f
	glUniform1f(inside_eff.get_uniform_location("fog_end"), 100.0f);
	// Set fog density: 0.04f
	glUniform1f(inside_eff.get_uniform_location("fog_density"), 0.04f);
	// Set fog type: FOG_EXP2
	glUniform1i(inside_eff.get_uniform_location("fog_type"), FOG_EXP2);
	renderer::render(rama);
	glEnable(GL_CULL_FACE);
}

void render_terrain_cube(effect terrain_eff,
	mesh cube_terrain,
	array<texture, 4> terrain_texs,
	vector<point_light> points, vector<spot_light> spots,
	shadow_map shadow, mat4 &LightProjectionMat,
	mat4 PV, mat4 V, vec3 cam_pos)
{
	// Bind effect
	renderer::bind(terrain_eff);
	// Set the texture value for the shader here
	glUniform1i(terrain_eff.get_uniform_location("tex[0]"), 0);
	glUniform1i(terrain_eff.get_uniform_location("tex[1]"), 1);
	glUniform1i(terrain_eff.get_uniform_location("tex[2]"), 2);
	glUniform1i(terrain_eff.get_uniform_location("tex[3]"), 3);
	// Find the lcoation for the MVP uniform
	const auto loc = terrain_eff.get_uniform_location("MVP");
	// Bind material
	renderer::bind(cube_terrain.get_material(), "mat");
	// Bind point light
	renderer::bind(points, "points");
	// Bind spot light
	renderer::bind(spots, "spots");
	// Bind textures to renderer
	renderer::bind(terrain_texs[0], 0);
	renderer::bind(terrain_texs[1], 1);
	renderer::bind(terrain_texs[2], 2);
	renderer::bind(terrain_texs[3], 3);
	// Set fog colour
	glUniform4fv(terrain_eff.get_uniform_location("fog_colour"), 1, value_ptr(vec4(0.412f, 1.0f, 0.996f, 1.0f)));
	// Set fog start:  5.0f
	glUniform1f(terrain_eff.get_uniform_location("fog_start"), 5.0f);
	// Set fog end:  100.0f
	glUniform1f(terrain_eff.get_uniform_location("fog_end"), 100.0f);
	// Set fog density: 0.04f
	glUniform1f(terrain_eff.get_uniform_location("fog_density"), 0.04f);
	// Set fog type: FOG_EXP2
	glUniform1i(terrain_eff.get_uniform_location("fog_type"), FOG_EXP2);
	// Set eye position
	glUniform3fv(terrain_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	auto M = cube_terrain.get_transform().get_transform_matrix();
	// Set MVP matrix uniform
	glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
	// Set M matrix uniform
	glUniformMatrix4fv(terrain_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set MV matrix uniform
	glUniformMatrix4fv(terrain_eff.get_uniform_location("MV"), 1, GL_FALSE, value_ptr(V * M));
	// Set N matrix uniform
	glUniformMatrix3fv(terrain_eff.get_uniform_location("N"), 1, GL_FALSE,
	value_ptr(cube_terrain.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	glUniformMatrix4fv(terrain_eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind shadow map texture
	renderer::bind(shadow.buffer->get_depth(), 4);
	// Set the shadow_map uniform
	glUniform1i(terrain_eff.get_uniform_location("shadow_map"), 4);
	// Render mesh
	renderer::render(cube_terrain);
}

// Render asteroid particles
void render_particles(effect compute_eff, effect eff,
					  const unsigned int MAX_PARTICLES, GLuint G_Position_buffer, GLuint G_Velocity_buffer,
					  mat4 M, mat4 PV)
{
	// Bind Compute Shader
	renderer::bind(compute_eff);
	// Bind data as SSBO
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, G_Position_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, G_Velocity_buffer);
	// Dispatch
	glDispatchCompute(MAX_PARTICLES / 128, 1, 1);
	// Sync, wait for completion
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	// Bind render effect
	renderer::bind(eff);
	// Create MVP matrix
	auto MVP = PV * M;
	// Set the colour uniform
	glUniform4fv(eff.get_uniform_location("colour"), 1, value_ptr(vec4(0.3f, 0.4f, 0.52f, 0.75f)));
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Bind position buffer as GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, G_Position_buffer);
	// Setup vertex format
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// Render
	glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
	// Tidy up
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}