// Solar system model - A simple interactive model of 
// the solar system with some spacecraft
// Last modified - 07/04/2017

#include <glm\glm.hpp>
#include <graphics_framework.h>
#include "cameras.h"
#include "render_helpers.h"
#include "solar_objects.h"
#include "spacecraft.h"
#include "lights.h"

using namespace std;
using namespace std::chrono;
using namespace graphics_framework;
using namespace glm;

// Maximum number of particles
const unsigned int MAX_PARTICLES = 5000;

vec4 positions[MAX_PARTICLES];
vec4 velocitys[MAX_PARTICLES];

GLuint G_Position_buffer, G_Velocity_buffer;

effect eff;
effect compute_eff;
GLuint vao;

map<string, mesh> solar_objects;
array<mesh, 7> enterprise;
array<mesh, 2> motions;
mesh rama;
array<mesh, 6> rama_terrain;
mesh shadow_plane;
mesh stars;
cubemap cube_map;

effect planet_eff;
effect skybox_eff;
effect cloud_eff;
effect sun_eff;
effect distortion_eff;
effect ship_eff;
effect inside_eff;
effect terrain_eff;
effect outside_eff;
effect shadow_eff;
effect tex_eff;
effect motion_blur;
effect cockpit_eff;
effect weather_eff;

map<string, texture> textures;
array<texture, 14> jupiter_texs;
array<texture, 4> terrain_texs;
map<string, texture> normal_maps;
array<texture, 2> motions_textures;

target_camera tcam;
free_camera fcam;
chase_camera ccam;
mesh target_mesh;
string target = "earth";
bool chase_camera_active = false;
bool free_camera_active = false;

vector<point_light> points(1);
vector<spot_light> spots(1);
vector<point_light> points_rama(1);
vector<spot_light> spots_rama(2);
shadow_map shadow;

map<string, float> orbit_factors;
double cursor_x = 0.0;
double cursor_y = 0.0;
float rotAngle = 0.0f;
bool destroy_solar_system = false;
bool demo_shadow = false;

frame_buffer frames[2];
frame_buffer temp_frame;
geometry screen_quad;
texture alpha_map;
unsigned int current_frame = 0;
float blur_factor = 0.9f;

mesh comet;
GLuint pvao;

float total_time;
float explode_factor = 0.0f;
float random = 0.0f;
default_random_engine generator;
uniform_int_distribution<int> distribution(1, 10);
uniform_real_distribution<float> distribution_y(-8, 8);
uniform_real_distribution<float> distribution_theta(0, 2 * pi<float>());
float peak_factor = 2.5f;
vec3 sun_activity;

effect blur;
effect dof;
frame_buffer first_pass;
frame_buffer temp_frames[2];

float weather_factor;

vec3 generate_random_sphere_point(float radius)
{
	float y = distribution_y(generator);
	float theta = distribution_theta(generator);
	float lat = asinf (y / radius);
	float x = radius * cosf(lat)*cosf(theta);
	float z = radius * cosf(lat)*sinf(theta);
	return vec3(x, y, z);
}

bool load_content() {
	// Create 2 frame buffers - use screen width and height
	temp_frames[0] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	temp_frames[1] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create a first_pass frame
	first_pass = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());

	// Load in blur
	blur.add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	blur.add_shader("shaders/blur.frag", GL_FRAGMENT_SHADER);
	// Load in depth of field effect
	dof.add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	dof.add_shader("shaders/depth_of_field.frag", GL_FRAGMENT_SHADER);
	blur.build();
	dof.build();

	// Create frame buffer - use screen width and height
	frames[0] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	frames[1] = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create a temporary frame buffer
	temp_frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create screen quad
	vector<vec3> screen_positions{vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f),
								  vec3(1.0f, 1.0f, 0.0f)};
	vector<vec2> screen_tex_coords{vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f)};
	screen_quad.add_buffer(screen_positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(screen_tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);

	alpha_map = texture("textures/cockpit.jpg");

	load_solar_objects(solar_objects, textures, jupiter_texs, normal_maps, orbit_factors);
	load_enterprise(enterprise, motions, textures, motions_textures, normal_maps);
	load_rama(rama, rama_terrain, textures, terrain_texs, normal_maps);
	load_shadow_plane(shadow_plane, textures, normal_maps);
	load_lights(points, spots, points_rama, spots_rama, rama.get_transform().position);
	load_cameras(tcam, fcam, ccam);
	glGenVertexArrays(1, &pvao);

	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());

	// SKYBOX
	stars = mesh(geometry_builder::create_box());
	stars.get_transform().scale = vec3(1000.0f);
	array<string, 6> filenames = { "textures/stars_ft.jpg", "textures/stars_bk.jpg", "textures/stars_up.jpg", "textures/stars_dn.jpg", "textures/stars_lt.jpg", "textures/stars_rt.jpg" };
	cube_map = cubemap(filenames);

	// SHADERS
	// Load in shaders for planets
	planet_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> planet_eff_frag_shaders{ "shaders/simple_texture.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_spot.frag", "shaders/part_fog.frag" };
	planet_eff.add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	planet_eff.build();

	// Load in shaders for clouds
	cloud_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> cloud_eff_frag_shaders{ "shaders/cloud_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_fog.frag" };
	cloud_eff.add_shader(cloud_eff_frag_shaders, GL_FRAGMENT_SHADER);
	cloud_eff.build();
	
	// Load in shaders for sun
	sun_eff.add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	sun_eff.add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	sun_eff.add_shader("shaders/explode.geom", GL_GEOMETRY_SHADER);
	sun_eff.build();

	// Load in shaders for black hole
	distortion_eff.add_shader("shaders/env_map.vert", GL_VERTEX_SHADER);
	distortion_eff.add_shader("shaders/env_map.frag", GL_FRAGMENT_SHADER);
	distortion_eff.build();

	// Load in shaders for enterprise
	ship_eff.add_shader("shaders/enterprise.vert", GL_VERTEX_SHADER);
	vector<string> ship_eff_frag_shaders{ "shaders/enterprise.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	ship_eff.add_shader(ship_eff_frag_shaders, GL_FRAGMENT_SHADER);
	ship_eff.build();

	// Load in shaders for rama
	inside_eff.add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	vector<string> inside_eff_frag_shaders{ "shaders/inside.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_fog.frag" };
	inside_eff.add_shader(inside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	inside_eff.build();

	terrain_eff.add_shader("shaders/terrain.vert", GL_VERTEX_SHADER);
	vector<string> terrain_eff_frag_shaders{ "shaders/terrain.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_weighted_texture_4.frag", "shaders/part_fog.frag" };
	terrain_eff.add_shader(terrain_eff_frag_shaders, GL_FRAGMENT_SHADER);
	terrain_eff.build();

	outside_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> outside_eff_frag_shaders{ "shaders/blend.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	outside_eff.add_shader(outside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	outside_eff.build();

	// Load in shaders for weather changes
	weather_eff.add_shader("shaders/weather.vert", GL_VERTEX_SHADER);
	vector<string> weather_eff_frag_shaders{ "shaders/weather.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag" };
	weather_eff.add_shader(weather_eff_frag_shaders, GL_FRAGMENT_SHADER);
	weather_eff.build();

	// Load in shaders for skybox
	skybox_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	vector<string> skybox_eff_frag_shaders {"shaders/skybox.frag", "shaders/part_fog.frag" };
	skybox_eff.add_shader(skybox_eff_frag_shaders, GL_FRAGMENT_SHADER);
	skybox_eff.build();

	// Load in shadow shaders
	shadow_eff.add_shader("shaders/spot.vert", GL_VERTEX_SHADER);
	shadow_eff.add_shader("shaders/spot.frag", GL_FRAGMENT_SHADER);
	shadow_eff.build();
	
	motion_blur.add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	motion_blur.add_shader("shaders/motion_blur.frag", GL_FRAGMENT_SHADER);
	motion_blur.build();

	tex_eff.add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	tex_eff.add_shader("shaders/screen.frag", GL_FRAGMENT_SHADER);
	tex_eff.build();

	cockpit_eff.add_shader("shaders/screen.vert", GL_VERTEX_SHADER);
	cockpit_eff.add_shader("shaders/mask.frag", GL_FRAGMENT_SHADER);
	cockpit_eff.build();

	// PARTICLES
	default_random_engine rand(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
	uniform_real_distribution<float> dist;

	// Initilise particles
	for (unsigned int i = 0; i < MAX_PARTICLES; ++i) {
		positions[i] = vec4(-(400.0f * dist(rand)), 40.0f * dist(rand) - 20.0f, 50.0f * dist(rand) - 15.0f, 0.0f);
		velocitys[i] = vec4(-(100.0f + (20.0f * dist(rand))), 0.0f, 0.0f, 0.0f);
	}

	// Load in shaders
	eff.add_shader("shaders/basic_colour.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/basic_colour.frag", GL_FRAGMENT_SHADER);
	eff.build();
	// Load in shaders
	compute_eff.add_shader("shaders/particle.comp", GL_COMPUTE_SHADER);
	compute_eff.build();

	// a useless vao, but we need it bound or we get errors.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//Generate Position Data buffer
	glGenBuffers(1, &G_Position_buffer);
	// Bind as GL_SHADER_STORAGE_BUFFER
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, G_Position_buffer);
	// Send Data to GPU, use GL_DYNAMIC_DRAW
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(positions[0]) * MAX_PARTICLES, positions, GL_DYNAMIC_DRAW);
	// Generate Velocity Data buffer
	glGenBuffers(1, &G_Velocity_buffer);
	// Bind as GL_SHADER_STORAGE_BUFFER
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, G_Velocity_buffer);
	// Send Data to GPU, use GL_DYNAMIC_DRAW
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(velocitys[0]) * MAX_PARTICLES, velocitys, GL_DYNAMIC_DRAW);
	//Unbind
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return true;
}

bool update(float delta_time) {
	// Flip frame
	current_frame = (current_frame + 1) % 2;

	if (delta_time > 10.0f) {
		delta_time = 10.0f;
	}

	// Accumulate time
	total_time += delta_time;

	// If random number between 1 and 10 is greater than 8
	// then increase the peak_factor, else decrease it till
	// it reaches it's minimum of 2.5
	if (random > 2.0f)
		peak_factor = std::min(peak_factor + 0.01f, 5.0f);
	else
		peak_factor = std::max(peak_factor - 0.01f, 2.5f);

	// Update the explode_factor - base on sin wave
	explode_factor = std::max(0.25f, abs(0.0f + sinf(total_time)));

	// Only update random when total_time modulo 10 is less than 0.5
	// This gives time for the change in explode factor to be noticed
	if (mod(total_time, 10.0f) <= 0.1f)
	{
		random = distribution(generator);
	}
	if (mod(total_time, 20.0f) <= 0.1f)
	{
		sun_activity = generate_random_sphere_point(8.0f);
		peak_factor = 0.0f;
	}

	// Update the weather_factor
	weather_factor += 0.01f;
	if (weather_factor > 0.99f)
		weather_factor = 0.0f;

	// USER CONTROLS
	// Camera controls
	// c - switch to chase camera
	if (glfwGetKey(renderer::get_window(), 'C'))
	{
		chase_camera_active = true;
		free_camera_active = false;
	}
	// t - switch to target camera
	if (glfwGetKey(renderer::get_window(), 'T'))
	{
		chase_camera_active = false;
		free_camera_active = false;
	}
	// f - switch to free camera
	if (glfwGetKey(renderer::get_window(), 'F'))
	{
		chase_camera_active = false;
		free_camera_active = true;
	}

	// Shadow plane controls
	if (glfwGetKey(renderer::get_window(), 'P'))
		demo_shadow = true;
	if (glfwGetKey(renderer::get_window(), 'O'))
		demo_shadow = false;

	// Check if solar system is to be destroyed
	if (destroy_solar_system == true)
		black_hole(solar_objects["sun"], solar_objects["black_hole"], solar_objects["distortion"], blur_factor, delta_time);

	// Enterprise controls
	vec3 engage;
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) 
		engage += vec3(0.0f, 0.0f, 1.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) 
		engage += vec3(0.0f, 0.0f, -1.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) 
		engage += vec3(-1.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) 
		engage += vec3(1.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_PAGE_UP))
		engage += vec3(0.0f, 1.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_PAGE_DOWN))
		engage += vec3(0.0f, -1.0f, 0.0f);
	// Move enterprise
	move_enterprise(enterprise, motions, engage, delta_time);
	// Spin rama
	spin_rama(rama, rama_terrain[0], delta_time);

	// ORBITS
	system_motion(solar_objects, orbit_factors, destroy_solar_system, delta_time);

	target_mesh = solar_objects[target];

	// Bind as GL_SHADER_STORAGE_BUFFER
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, G_Velocity_buffer);
	// Send Data to GPU, use GL_DYNAMIC_DRAW
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(velocitys[0]) * MAX_PARTICLES, velocitys, GL_DYNAMIC_DRAW);

	renderer::bind(compute_eff);
	glUniform1f(compute_eff.get_uniform_location("delta_time"), delta_time);
	glUniform3fv(compute_eff.get_uniform_location("max_dims"), 1, value_ptr(vec3(500.0f, 100.0f, 100.0f)));

	// CAMERA MODES
	// Update depending on active camera
	update_active_camera(chase_camera_active, free_camera_active,
		ccam, fcam, tcam, stars, target_mesh, delta_time, cursor_x, cursor_y);

	// Check for selection
	// If mouse button pressed get ray and check for intersection
	if (glfwGetMouseButton(renderer::get_window(), GLFW_MOUSE_BUTTON_LEFT))
	{
		// Get the mouse position
		double mouse_x;
		double mouse_y;
		glfwGetCursorPos(renderer::get_window(), &mouse_x, &mouse_y);
		double xx = 2 * mouse_x / renderer::get_screen_width() - 1.0f;
		double yy = 2 * (renderer::get_screen_height() - mouse_y) / renderer::get_screen_height() - 1.0f;
		// Origin and direction of the ray
		vec4 origin;
		vec4 direction;
		// Convert mouse position to ray
		vec4 ray_start_screen(xx, yy, -1, 1);
		vec4 ray_end_screen(xx, yy, 0, 1);
		// Get inverse of PV
		auto P = tcam.get_projection();
		auto V = tcam.get_view();
		auto inverse_matrix = inverse(P * V);
		// Calculate origin and direction
		vec4 ray_start_world = inverse_matrix * ray_start_screen;
		ray_start_world = ray_start_world / ray_start_world.w;
		vec4 ray_end_world = inverse_matrix * ray_end_screen;
		ray_end_world = ray_end_world / ray_end_world.w;
		direction = normalize(ray_end_world - ray_start_world);
		origin = ray_start_world;
		// Check all the solar_objects for intersection
		for (auto &m : solar_objects)
		{
			float distance = 0.0f;
			if (test_ray_oobb(origin, direction, m.second.get_minimal(), m.second.get_maximal(),
				m.second.get_transform().get_transform_matrix(), distance))
			{
				if (m.first == "sun")
					destroy_solar_system = true;
				else
					target = m.first;
			}
		}
	}
	// SHADOW MAP UPDATE
	shadow.light_position = spots[0].get_position();
	shadow.light_dir = spots[0].get_direction();
	
	// FPS
	cout << "FPS: " << 1.0f / delta_time << endl;
	return true;
}

bool render() {
	// Get view and projection matrices from active camera
	mat4 V;
	mat4 P;
	vec3 cam_pos;
	if (chase_camera_active)
	{
		cam_pos = ccam.get_position();
		V = ccam.get_view();
		P = ccam.get_projection();
	}
	else if (free_camera_active)
	{
		cam_pos = fcam.get_position();
		V = fcam.get_view();
		P = fcam.get_projection();
	}
	else
	{
		cam_pos = tcam.get_position();
		V = tcam.get_view();
		P = tcam.get_projection();
	}

	// Render to shadow map
	mat4 LightProjectionMat;
	create_shadow_map(shadow_eff,
		solar_objects, enterprise, motions, rama,
		shadow, LightProjectionMat);

	frame_buffer last_pass;
	if (!chase_camera_active)
	{
		// MOTION BLUR
		// Render to frame buffer
		renderer::set_render_target(temp_frame);
		// Clear frame
		renderer::clear();
		// Render background
		render_skybox(skybox_eff, stars, cube_map, P, V);
		// Render solar objects
		for (auto &e : solar_objects) {
			// Skip those: with scale of 0 (sucked into black hole),
			//		       not to be rendered unless sun has been clicked
			if (e.second.get_transform().scale == vec3(0.0f) ||
				((e.first == "distortion" || e.first == "black_hole") && destroy_solar_system == false))
			{
				continue;
			}
			// Render clouds (if the earth hasn't been sucked into the black hole)
			else if (e.first == "clouds" && solar_objects["earth"].get_transform().scale != vec3(0.0f))
			{
				render_clouds(cloud_eff,
					solar_objects["clouds"],
					textures["cloudsTex"], normal_maps["clouds"],
					points,
					shadow, LightProjectionMat,
					P, V, cam_pos);
					
			}
			// Render sun
			else if (e.first == "sun")
			{
				glDisable(GL_CULL_FACE);
				render_solar_objects(sun_eff,
					solar_objects[e.first],
					textures[e.first + "Tex"], normal_maps[e.first],
					points, spots,
					shadow, LightProjectionMat,
					P, V, cam_pos, explode_factor, peak_factor, sun_activity);
				glEnable(GL_CULL_FACE);
			}
			// Render distortion if necessary
			else if (e.first == "distortion")
			{
				render_distortion(distortion_eff,
					e.second,
					cube_map,
					P, V, cam_pos);
			}
			// Render Jupiter
			else if (e.first == "jupiter")
			{
				render_jupiter(weather_eff,
					e.second,
					jupiter_texs,
					points, spots, 
					shadow, LightProjectionMat, 
					P, V, cam_pos, weather_factor);
			}
			// Render planets
			else
			{
				render_solar_objects(planet_eff,
					solar_objects[e.first],
					textures[e.first + "Tex"], normal_maps[e.first],
					points, spots,
					shadow, LightProjectionMat,
					P, V, cam_pos, explode_factor, total_time, sun_activity);
			}
		}
		// Render the Enterprise
		render_enterprise(ship_eff,
			enterprise, motions,
			textures["enterprise"], normal_maps["saucer"], motions_textures,
			points, spots,
			shadow, LightProjectionMat,
			P, V, cam_pos);
		// Render fire (currently makes the previously renderered object disappear???)
		/*render_fire(compute_eff, smoke_eff,
			smoke_M,
			smoke,
			G_Position_buffer, G_Velocity_buffer, MAX_PARTICLES,
			P, V, cam_pos);*/
			// Render rama
		render_rama(outside_eff, inside_eff, terrain_eff,
			rama, rama_terrain,
			textures["ramaInTex"], textures["ramaOutTex"], textures["ramaGrassTex"], textures["blend_map"], normal_maps["ramaOut"], normal_maps["earth"], solar_objects["earth"].get_material(),
			terrain_texs,
			points, spots, points_rama, spots_rama,
			shadow, LightProjectionMat,
			P, V, cam_pos);
		// Render shadow plane if necessary
		if (demo_shadow == true)
			render_solar_objects(planet_eff,
				shadow_plane,
				textures["planeTex"], normal_maps["plane"],
				points, spots,
				shadow, LightProjectionMat,
				P, V, cam_pos, explode_factor, total_time, sun_activity);
		// Render comet
		render_solar_objects(planet_eff, solar_objects["comet"], textures["cometTex"], normal_maps["comet"], points, spots, shadow, LightProjectionMat, P, V, cam_pos, explode_factor, total_time, sun_activity);
		// Render comet particles
		glBindVertexArray(pvao);
		render_particles(compute_eff, eff, MAX_PARTICLES, G_Position_buffer, G_Velocity_buffer, solar_objects["comet"].get_transform().get_transform_matrix(), P, V);
		glBindVertexArray(0);

		// Set render target to current frame
		renderer::set_render_target(frames[current_frame]);
		// Clear frame
		renderer::clear();
		// Bind motion blur effect
		renderer::bind(motion_blur);
		// MVP is now the identity matrix
		mat4 MVP(1.0f);
		// Set MVP matrix uniform
		glUniformMatrix4fv(motion_blur.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind tempframe to TU 0.
		renderer::bind(temp_frame.get_frame(), 0);
		// Bind frames[(current_frame + 1) % 2] to TU 1.
		renderer::bind(frames[(current_frame + 1) % 2].get_frame(), 1);
		// Set tex uniforms
		glUniform1i(motion_blur.get_uniform_location("previous_frame"), 0);
		glUniform1i(motion_blur.get_uniform_location("tex"), 1);
		// Set blur factor
		glUniform1f(motion_blur.get_uniform_location("blend_factor"), blur_factor);
		// Render screen quad
		renderer::render(screen_quad);

		// Set render target back to the screen
		renderer::set_render_target();
		// Clear frame
		renderer::clear();
		if (free_camera_active)
		{
			// Bind Cockpit effect
			renderer::bind(cockpit_eff);
			// Set MVP matrix uniform
			glUniformMatrix4fv(cockpit_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			// Bind texture from frame buffer
			renderer::bind(frames[current_frame].get_frame(), 0);
			// Set the tex uniform
			glUniform1i(cockpit_eff.get_uniform_location("tex"), 0);
			// Bind alpha map
			renderer::bind(alpha_map, 1);
			// Set the alpha map uniform
			glUniform1i(cockpit_eff.get_uniform_location("alpha_map"), 1);
		}
		else
		{
			// Bind Tex effect
			renderer::bind(tex_eff);
			// Set MVP matrix uniform
			glUniformMatrix4fv(tex_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			// Bind texture from frame buffer
			renderer::bind(frames[current_frame].get_frame(), 0);
			// Set the tex uniform
			glUniform1i(tex_eff.get_uniform_location("tex"), 0);
		}
	}
	else
	{
		// CHASE CAMERA BLUR
		// !!!!!!!!!!!!!!! FIRST PASS !!!!!!!!!!!!!!!!
		// *********************************
		// Set render target to first_pass
		renderer::set_render_target(first_pass);
		// Clear frame
		renderer::clear();
		// *********************************
		// Render background
		render_skybox(skybox_eff, stars, cube_map, P, V);
		// Render solar objects
		for (auto &e : solar_objects) {
			// Skip those: with scale of 0 (sucked into black hole),
			//		       not to be rendered unless sun has been clicked
			if (e.second.get_transform().scale == vec3(0.0f) ||
				((e.first == "distortion" || e.first == "black_hole") && destroy_solar_system == false))
			{
				continue;
			}
			// Render clouds (if the earth hasn't been sucked into the black hole)
			else if (e.first == "clouds" && solar_objects["earth"].get_transform().scale != vec3(0.0f))
				render_clouds(cloud_eff,
					solar_objects["clouds"],
					textures["cloudsTex"], normal_maps["clouds"],
					points,
					shadow, LightProjectionMat,
					P, V, cam_pos);
			// Render sun
			else if (e.first == "sun")
			{
				glDisable(GL_CULL_FACE);
				render_solar_objects(sun_eff,
					solar_objects[e.first],
					textures[e.first + "Tex"], normal_maps[e.first],
					points, spots,
					shadow, LightProjectionMat,
					P, V, cam_pos, explode_factor, peak_factor, sun_activity);
				glEnable(GL_CULL_FACE);
			}
			// Render distortion if necessary
			else if (e.first == "distortion")
			{
				render_distortion(distortion_eff,
					e.second,
					cube_map,
					P, V, cam_pos);
			}
			else if (e.first == "jupiter")
			{
				render_jupiter(weather_eff,
					e.second,
					jupiter_texs,
					points, spots,
					shadow, LightProjectionMat,
					P, V, cam_pos, weather_factor);
			}
			// Render planets
			else
				render_solar_objects(planet_eff,
					solar_objects[e.first],
					textures[e.first + "Tex"], normal_maps[e.first],
					points, spots,
					shadow, LightProjectionMat,
					P, V, cam_pos, explode_factor, total_time, sun_activity);
		}
		// Render the Enterprise
		render_enterprise(ship_eff,
			enterprise, motions,
			textures["enterprise"], normal_maps["saucer"], motions_textures,
			points, spots,
			shadow, LightProjectionMat,
			P, V, cam_pos);
		// Render fire (currently makes the previously renderered object disappear???)
		/*render_fire(compute_eff, smoke_eff,
		smoke_M,
		smoke,
		G_Position_buffer, G_Velocity_buffer, MAX_PARTICLES,
		P, V, cam_pos);*/
		// Render rama
		render_rama(outside_eff, inside_eff, terrain_eff,
			rama, rama_terrain,
			textures["ramaInTex"], textures["ramaOutTex"], textures["ramaGrassTex"], textures["blend_map"], normal_maps["ramaOut"], normal_maps["earth"], solar_objects["earth"].get_material(),
			terrain_texs,
			points, spots, points_rama, spots_rama,
			shadow, LightProjectionMat,
			P, V, cam_pos);
		// Render shadow plane if necessary
		if (demo_shadow == true)
			render_solar_objects(planet_eff,
				shadow_plane,
				textures["planeTex"], normal_maps["plane"],
				points, spots,
				shadow, LightProjectionMat,
				P, V, cam_pos, explode_factor, total_time, sun_activity);
		// Render comet
		render_solar_objects(planet_eff, solar_objects["comet"], textures["cometTex"], normal_maps["comet"], points, spots, shadow, LightProjectionMat, P, V, cam_pos, explode_factor, total_time, sun_activity);
		// Render comet particles
		glBindVertexArray(pvao);
		render_particles(compute_eff, eff, MAX_PARTICLES, G_Position_buffer, G_Velocity_buffer, solar_objects["comet"].get_transform().get_transform_matrix(), P, V);
		glBindVertexArray(0);

		// !!!!!!!!!!!!!!! SECOND PASS !!!!!!!!!!!!!!!!

		last_pass = first_pass;


		// *********************************
		// Perform blur twice
		for (int i = 0; i < 2; i++)
		{
			// Set render target to temp_frames[i]
			renderer::set_render_target(temp_frames[i]);
			// Clear frame
			renderer::clear();
			// Bind motion blur effect
			renderer::bind(blur);
			// MVP is now the identity matrix
			mat4 MVP(1.0f);
			// Set MVP matrix uniform
			glUniformMatrix4fv(blur.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			// Bind frames
			renderer::bind(last_pass.get_frame(), 0);
			// Set inverse width
			glUniform1f(blur.get_uniform_location("inverse_width"), 1.0f / renderer::get_screen_width());
			// Set inverse height
			glUniform1f(blur.get_uniform_location("inverse_height"), 1.0f / renderer::get_screen_height());
			// Render screen quad
			renderer::render(screen_quad);
			// Set last pass to this pass
			last_pass = temp_frames[i];
		}
		// Set render target back to the screen
		renderer::set_render_target();
		// Clear frame
		renderer::clear();
		//Bid Dof effect
		renderer::bind(dof);
		// Set MVP matrix uniform, identity
		mat4 MVP(1.0f);
		glUniformMatrix4fv(dof.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind texture from last pass, 0
		renderer::bind(last_pass.get_frame(), 0);
		// Set the uniform, 0
		glUniform1i(dof.get_uniform_location("tex"), 0);
		// Sharp texture is taken from first pass
		// bind first pass, 1
		renderer::bind(first_pass.get_frame(), 1);
		//set sharp tex uniform, 1
		glUniform1i(dof.get_uniform_location("sharp"), 1);
		// Depth also taken from first pass
		// bind first pass **depth** to  TU 2
		renderer::bind(first_pass.get_depth(), 2);
		//set depth tex uniform, 2
		glUniform1i(dof.get_uniform_location("depth"), 2);
		// Set range and focus values
		// - range distance to chaser (get from camera)
		// - focus 0.07f
		glUniform1f(dof.get_uniform_location("range"), distance(ccam.get_position(), solar_objects["earth"].get_transform().position));
		glUniform1f(dof.get_uniform_location("focus"), 0.07f);
	}
	// Render the screen quad
	renderer::render(screen_quad);
	return true;
}

void main() {
	// Create application
	app application("Graphics Coursework");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}