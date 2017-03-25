// Solar system model - A simple interactive model of 
// the solar system with some spacecraft
// Last modified - 24/03/2017

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

const unsigned int MAX_PARTICLES = 4096;

vec4 positions[MAX_PARTICLES];
vec4 velocitys[MAX_PARTICLES];
GLuint G_Position_buffer, G_Velocity_buffer;
effect smoke_eff;
effect compute_eff;
GLuint vao;
texture smoke;
mat4 smoke_M;

map<string, mesh> solar_objects;
array<mesh, 7> enterprise;
array<mesh, 2> motions;
mesh rama;
mesh shadow_plane;
mesh stars;
cubemap cube_map;

effect planet_eff;
effect skybox_eff;
effect cloud_eff;
effect sun_eff;
effect ship_eff;
effect inside_eff;
effect outside_eff;
effect shadow_eff;

map<string, texture> textures;
map<string, texture> normal_maps;
array<texture, 2> motions_textures;

target_camera tcam;
free_camera fcam;
chase_camera ccam;
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

bool load_content() {
	load_solar_objects(solar_objects, textures, normal_maps, orbit_factors);
	load_enterprise(enterprise, motions, textures, motions_textures, normal_maps);
	load_rama(rama, textures, normal_maps);
	load_shadow_plane(shadow_plane, textures, normal_maps);
	load_lights(points, spots, points_rama, spots_rama, rama.get_transform().position);
	load_cameras(tcam, fcam, ccam);

	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());

	// SKYBOX
	stars = mesh(geometry_builder::create_box());
	stars.get_transform().scale = vec3(1000.0f);
	array<string, 6> filenames = { "textures/stars_ft.jpg", "textures/stars_bk.jpg", "textures/stars_up.jpg", "textures/stars_dn.jpg", "textures/stars_lt.jpg", "textures/stars_rt.jpg" };
	cube_map = cubemap(filenames);

	// SHADERS
	// Load in shaders for planets
	planet_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> planet_eff_frag_shaders{ "shaders/simple_texture.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_spot.frag" };
	planet_eff.add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	planet_eff.build();
	
	// Load in shaders for clouds
	cloud_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> cloud_eff_frag_shaders{ "shaders/cloud_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag"};
	cloud_eff.add_shader(cloud_eff_frag_shaders, GL_FRAGMENT_SHADER);
	cloud_eff.build();
	
	// Load in shaders for sun
	sun_eff.add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	sun_eff.add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	sun_eff.build();

	// Load in shaders for enterprise
	ship_eff.add_shader("shaders/enterprise.vert", GL_VERTEX_SHADER);
	vector<string> ship_eff_frag_shaders{ "shaders/enterprise.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	ship_eff.add_shader(ship_eff_frag_shaders, GL_FRAGMENT_SHADER);
	ship_eff.build();

	// Load in shaders for rama
	inside_eff.add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	vector<string> inside_eff_frag_shaders{ "shaders/inside.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	inside_eff.add_shader(inside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	inside_eff.build();

	// Load in shaders for rama
	outside_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> outside_eff_frag_shaders{ "shaders/blend.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	outside_eff.add_shader(outside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	outside_eff.build();

	// Load in shaders for skybox
	skybox_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	skybox_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	skybox_eff.build();

	// Load in shadow shaders
	shadow_eff.add_shader("shaders/spot.vert", GL_VERTEX_SHADER);
	shadow_eff.add_shader("shaders/spot.frag", GL_FRAGMENT_SHADER);
	shadow_eff.build();
	
	// PARTICLES
	default_random_engine rand(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
	uniform_real_distribution<float> dist;
	smoke_M = enterprise[0].get_transform().get_transform_matrix();
	smoke = texture("textures/smoke.png");
	// Initilise particles
	for (unsigned int i = 0; i < MAX_PARTICLES; ++i) {
		positions[i] = vec4(((2.0f * dist(rand)) - 1.0f) / 10.0f, 5.0f * dist(rand), 0.0f, 0.0f);
		velocitys[i] = vec4(0.0f, 0.1f + dist(rand), 0.0f, 0.0f);
	}
	// Load in shaders
	smoke_eff.add_shader("shaders/smoke.vert", GL_VERTEX_SHADER);
	smoke_eff.add_shader("shaders/smoke.frag", GL_FRAGMENT_SHADER);
	smoke_eff.add_shader("shaders/smoke.geom", GL_GEOMETRY_SHADER);
	smoke_eff.build();
	// Load in shaders
	compute_eff.add_shader("shaders/particle.comp", GL_COMPUTE_SHADER);
	compute_eff.build();
	// a useless vao, but we need it bound or we get errors.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// *********************************
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
	// *********************************
	//Unbind
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	return true;
}

bool update(float delta_time) {
	if (delta_time > 10.0f) {
		delta_time = 10.0f;
	}
	
	renderer::bind(compute_eff);
	glUniform3fv(compute_eff.get_uniform_location("max_dims"), 1, &(vec3(3.0f, 5.0f, 5.0f))[0]);
	glUniform1f(compute_eff.get_uniform_location("delta_time"), delta_time);
	
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
		black_hole(solar_objects["sun"], solar_objects["black_hole"], delta_time);

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
	spin_rama(rama, delta_time);

	// ORBITS
	system_motion(solar_objects, orbit_factors, destroy_solar_system, delta_time);

	// CAMERA MODES
	// Update depending on active camera
	update_active_camera(chase_camera_active, free_camera_active,
		ccam, fcam, tcam, stars, solar_objects["earth"], delta_time, cursor_x, cursor_y);

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
		// **Currently only used to check for a sun click
		// will implement a planet view selection**
		for (auto &m : solar_objects)
		{
			float distance = 0.0f;
			if (test_ray_oobb(origin, direction, m.second.get_minimal(), m.second.get_maximal(),
				m.second.get_transform().get_transform_matrix(), distance))
			{
				if (m.first == "sun")
					destroy_solar_system = true;
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
	// Render background
	render_skybox(skybox_eff, stars, cube_map, P, V);
	// Render to shadow map
	mat4 LightProjectionMat;
	create_shadow_map(shadow_eff,
					  solar_objects, enterprise, motions, rama, 
					  shadow, LightProjectionMat);
	// Render solar objects
	for (auto &e : solar_objects) {
		// Skip those: with scale of 0 (sucked into black hole),
		//		       not to be rendered unless sun has been clicked
		if (e.second.get_transform().scale == vec3(0.0f) ||
			(e.first == "black_hole" && destroy_solar_system == false))
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
			render_solar_objects(sun_eff, 
								 solar_objects[e.first], 
								 textures[e.first + "Tex"], normal_maps[e.first],
							     points, spots,
								 shadow, LightProjectionMat,
								 P, V, cam_pos);
		// Render planets
		else
			render_solar_objects(planet_eff, 
								 solar_objects[e.first], 
								 textures[e.first + "Tex"], normal_maps[e.first],
								 points, spots, 
								 shadow, LightProjectionMat, 
								 P, V, cam_pos);
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
	render_rama(outside_eff, inside_eff, 
				rama, 
				textures["ramaInTex"], textures["ramaOutTex"], textures["ramaGrassTex"], textures["blend_map"], normal_maps["ramaOut"], normal_maps["earth"], solar_objects["earth"].get_material(), 
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
							 P, V, cam_pos);
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