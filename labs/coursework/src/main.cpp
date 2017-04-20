// Solar system model - A simple interactive model of 
// the solar system with some spacecraft
// Last modified - 20/04/2017

#include <glm\glm.hpp>
#include <graphics_framework.h>
#include "cameras.h"
#include "render_helpers.h"
#include "solar_objects.h"
#include "spacecraft.h"
#include "lights.h"
#include "post_processing.h"

using namespace std;
using namespace std::chrono;
using namespace graphics_framework;
using namespace glm;

//Effects
map<string, effect> effects;

// Meshes
map<string, mesh> solar_objects;
array<mesh, 7> enterprise;
array<mesh, 2> motions;
mesh rama;
array<mesh, 6> rama_terrain;
mesh cube_terrain;
mesh stars;

// Particles
const unsigned int MAX_PARTICLES = 5000;
vec4 positions[MAX_PARTICLES];
vec4 velocitys[MAX_PARTICLES];
GLuint G_Position_buffer, G_Velocity_buffer;
effect eff;
effect compute_eff;
GLuint vao;
GLuint pvao;

// Textures
map<string, texture> textures;
array<texture, 14> jupiter_texs;
array<texture, 4> terrain_texs;
map<string, texture> normal_maps;
array<texture, 2> motions_textures;
cubemap cube_map;

// Cameras
target_camera tcam;
free_camera fcam;
chase_camera ccam;
mesh target_mesh;
string target = "earth";
bool chase_camera_active = false;
bool free_camera_active = false;

// Lights
vector<point_light> points(1);
vector<spot_light> spots(1);
vector<point_light> points_rama(1);
vector<spot_light> spots_rama(2);
shadow_map shadow;

// System motion
map<string, float> orbit_factors;
double cursor_x = 0.0;
double cursor_y = 0.0;
float rotAngle = 0.0f;
bool destroy_solar_system = false;
bool demo_shadow = false;
float weather_factor;

// Post processing
array<frame_buffer, 2> frames;
frame_buffer temp_frame;
geometry screen_quad;
texture alpha_map;
unsigned int current_frame = 0;
float blur_factor = 0.9f;

frame_buffer first_pass;
array<frame_buffer, 2> temp_frames;

// Solar activity
float total_time;
float explode_factor = 0.0f;
float random = 0.0f;
float peak_factor = 2.5f;
vec3 sun_activity;

// Black hole distortion
geometry distortion;
float distortion_size = 1.0f;

// Buckets
vector<string> planet_eff = { "mercury", "venus", "earth", "mars", "comet", "shadow_plane", "black_hole" };

void render_whole_scene(mat4 P, mat4 V, mat4 LightProjectionMat, vec3 cam_pos)
{
	// SET CONSTANT PV VALUE TO SAVE COMPUTING FOR EVERY OBJECT
	const auto PV = P * V;

	// RENDER SKYBOX
	render_skybox(effects["skybox_eff"], stars, cube_map, PV);

	// RENDER PLANET_EFF OBJECTS
	// Bind common resources
	renderer::bind(effects["planet_eff"]);
	// Bind lights
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	// Set eye position
	glUniform3fv(effects["planet_eff"].get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind shadow map texture
	renderer::bind(shadow.buffer->get_depth(), 2);
	// Set the shadow_map uniform
	glUniform1i(effects["planet_eff"].get_uniform_location("shadow_map"), 2);

	// Render solar objects
	for (auto &e : solar_objects)
	{
		// Skip those: with scale of 0 (sucked into black hole),
		//		       not to be rendered unless sun has been clicked,
		//			   not to be rendererd with planet_eff
		if (e.second.get_transform().scale == vec3(0.0f) ||
			(e.first == "black_hole" && destroy_solar_system == false) ||
			(find(planet_eff.begin(), planet_eff.end(), e.first) == planet_eff.end()))
		{
			continue;
		}
		auto &m = e.second;
		render_solar_objects(effects["planet_eff"],
			m, 
			textures[e.first + "Tex"], normal_maps[e.first],
			shadow,
			LightProjectionMat, PV);
	}
	// Render terrain if necessary
	if (demo_shadow == true)
	{
		render_terrain_cube(effects["terrain_eff"], cube_terrain, terrain_texs, points, spots, shadow, LightProjectionMat, PV, V, cam_pos);
	}

	// RENDER THE REST OF THE OBJECTS
	// Sun
	render_sun(effects["sun_eff"], 
		solar_objects["sun"], 
		textures["sunTex"], normal_maps["sun"],
		points, spots, 
		shadow, LightProjectionMat, 
		PV, cam_pos, 
		explode_factor, peak_factor, sun_activity);

	// Clouds
	render_clouds(effects["cloud_eff"],
		solar_objects["clouds"],
		textures["cloudsTex"], normal_maps["clouds"],
		points,
		PV, cam_pos);

	// Jupiter
	render_jupiter(effects["weather_eff"],
		solar_objects["jupiter"],
		jupiter_texs,
		points, spots,
		shadow, LightProjectionMat,
		PV, cam_pos, weather_factor);

	// Comet particles
	glBindVertexArray(pvao);
	render_particles(compute_eff, eff, MAX_PARTICLES, G_Position_buffer, G_Velocity_buffer, solar_objects["comet"].get_transform().get_transform_matrix(), PV);
	glBindVertexArray(0);

	// Enterprise
	render_enterprise(effects["ship_eff"],
		enterprise, motions,
		textures["enterprise"], normal_maps["saucer"], motions_textures,
		points, spots,
		shadow, LightProjectionMat,
		PV, cam_pos);

	// Rama
	render_rama(effects["outside_eff"], effects["inside_eff"], effects["terrain_eff"],
		rama, rama_terrain,
		textures["ramaInTex"], textures["ramaOutTex"], textures["ramaGrassTex"], textures["blend_map"], normal_maps["ramaOut"], normal_maps["earth"], solar_objects["earth"].get_material(),
		terrain_texs,
		points, spots, points_rama, spots_rama,
		shadow, LightProjectionMat,
		PV, V, cam_pos);

	// Distortion
	if (destroy_solar_system)
	{
		renderer::bind(effects["distortion_eff"]);
		glUniformMatrix4fv(effects["distortion_eff"].get_uniform_location("MV"), 1, GL_FALSE, value_ptr(V));
		glUniformMatrix4fv(effects["distortion_eff"].get_uniform_location("P"), 1, GL_FALSE, value_ptr(P));
		glUniform1f(effects["distortion_eff"].get_uniform_location("point_size"), distortion_size);
		renderer::bind(cube_map, 0);
		glUniform1i(effects["distortion_eff"].get_uniform_location("tex"), 0);
		renderer::render(distortion);
	}
}

bool load_content() {
	load_post_processing(temp_frames, first_pass, frames, temp_frame, screen_quad, alpha_map, effects);
	load_solar_objects(solar_objects, distortion, textures, jupiter_texs, normal_maps, orbit_factors, effects);
	load_enterprise(enterprise, motions, textures, motions_textures, normal_maps, effects);
	load_rama(rama, rama_terrain, textures, terrain_texs, normal_maps, effects);
	load_terrain(cube_terrain, terrain_texs, effects);
	load_lights(points, spots, points_rama, spots_rama, rama.get_transform().position);
	load_cameras(tcam, fcam, ccam);
	glGenVertexArrays(1, &pvao);

	// SKYBOX
	stars = mesh(geometry_builder::create_box());
	stars.get_transform().scale = vec3(1000.0f);
	array<string, 6> filenames = { "textures/stars_ft.jpg", "textures/stars_bk.jpg", "textures/stars_up.jpg", "textures/stars_dn.jpg", "textures/stars_lt.jpg", "textures/stars_rt.jpg" };
	cube_map = cubemap(filenames);

	// Load in shaders for skybox
	effects["skybox_eff"].add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	vector<string> skybox_eff_frag_shaders {"shaders/skybox.frag", "shaders/part_fog.frag" };
	effects["skybox_eff"].add_shader(skybox_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["skybox_eff"].build();

	// SHADOWS
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
	// Load in shadow shaders
	effects["shadow_eff"].add_shader("shaders/spot.vert", GL_VERTEX_SHADER);
	effects["shadow_eff"].add_shader("shaders/spot.frag", GL_FRAGMENT_SHADER);
	effects["shadow_eff"].build();
	
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

	// Accumulate time
	total_time += delta_time;

	// DYNAMIC CHANGES
	// Control solar activity
	solar_activity(random, peak_factor, explode_factor, sun_activity, total_time);
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
		black_hole(solar_objects["sun"], solar_objects["black_hole"], distortion_size, blur_factor, delta_time);

	// SPACECRAFT
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

	// Update target mesh
	target_mesh = solar_objects[target];

	// COMET PARTICLES
	// Bind as GL_SHADER_STORAGE_BUFFER
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, G_Velocity_buffer);
	// Send Data to GPU, use GL_DYNAMIC_DRAW
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(velocitys[0]) * MAX_PARTICLES, velocitys, GL_DYNAMIC_DRAW);
	renderer::bind(compute_eff);
	glUniform1f(compute_eff.get_uniform_location("delta_time"), std::min(delta_time, 10.0f));
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
	create_shadow_map(effects["shadow_eff"],
		solar_objects, enterprise, motions, rama,
		shadow, LightProjectionMat);

	// For target and free camera, perform motion blur
	frame_buffer last_pass;
	if (!chase_camera_active)
	{
		// MOTION BLUR
		// Render to frame buffer
		renderer::set_render_target(temp_frame);
		// Clear frame
		renderer::clear();
		// Render the scene
		render_whole_scene(P, V, LightProjectionMat, cam_pos);
		// Set render target to current frame
		renderer::set_render_target(frames[current_frame]);
		// Clear frame
		renderer::clear();
		// Bind motion blur effect
		renderer::bind(effects["motion_blur"]);
		// MVP is now the identity matrix
		mat4 MVP(1.0f);
		// Set MVP matrix uniform
		glUniformMatrix4fv(effects["motion_blur"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind tempframe to TU 0.
		renderer::bind(temp_frame.get_frame(), 0);
		// Bind frames[(current_frame + 1) % 2] to TU 1.
		renderer::bind(frames[(current_frame + 1) % 2].get_frame(), 1);
		// Set tex uniforms
		glUniform1i(effects["motion_blur"].get_uniform_location("previous_frame"), 0);
		glUniform1i(effects["motion_blur"].get_uniform_location("tex"), 1);
		// Set blur factor
		glUniform1f(effects["motion_blur"].get_uniform_location("blend_factor"), blur_factor);
		// Render screen quad
		renderer::render(screen_quad);

		// Set render target back to the screen
		renderer::set_render_target();
		// Clear frame
		renderer::clear();
		// For free camera, perform masking as well
		if (free_camera_active)
		{
			// Bind Cockpit effect
			renderer::bind(effects["cockpit_eff"]);
			// Set MVP matrix uniform
			glUniformMatrix4fv(effects["cockpit_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			// Bind texture from frame buffer
			renderer::bind(frames[current_frame].get_frame(), 0);
			// Set the tex uniform
			glUniform1i(effects["cockpit_eff"].get_uniform_location("tex"), 0);
			// Bind alpha map
			renderer::bind(alpha_map, 1);
			// Set the alpha map uniform
			glUniform1i(effects["cockpit_eff"].get_uniform_location("alpha_map"), 1);
		}
		// For target, just the motion blur
		else
		{
			// Bind Tex effect
			renderer::bind(effects["tex_eff"]);
			// Set MVP matrix uniform
			glUniformMatrix4fv(effects["tex_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			// Bind texture from frame buffer
			renderer::bind(frames[current_frame].get_frame(), 0);
			// Set the tex uniform
			glUniform1i(effects["tex_eff"].get_uniform_location("tex"), 0);
		}
	}
	// For chase camera, perform depth of field blur
	else
	{
		// CHASE CAMERA BLUR
		// FIRST PASS
		// Set render target to first_pass
		renderer::set_render_target(first_pass);
		// Clear frame
		renderer::clear();
		// Render the scene
		render_whole_scene(P, V, LightProjectionMat, cam_pos);
		// SECOND PASS
		last_pass = first_pass;
		// Perform blur twice
		for (int i = 0; i < 2; i++)
		{
			// Set render target to temp_frames[i]
			renderer::set_render_target(temp_frames[i]);
			// Clear frame
			renderer::clear();
			// Bind motion blur effect
			renderer::bind(effects["blur"]);
			// MVP is now the identity matrix
			mat4 MVP(1.0f);
			// Set MVP matrix uniform
			glUniformMatrix4fv(effects["blur"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
			// Bind frames
			renderer::bind(last_pass.get_frame(), 0);
			// Set inverse width
			glUniform1f(effects["blur"].get_uniform_location("inverse_width"), 1.0f / renderer::get_screen_width());
			// Set inverse height
			glUniform1f(effects["blur"].get_uniform_location("inverse_height"), 1.0f / renderer::get_screen_height());
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
		renderer::bind(effects["dof"]);
		// Set MVP matrix uniform, identity
		mat4 MVP(1.0f);
		glUniformMatrix4fv(effects["dof"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Bind texture from last pass, 0
		renderer::bind(last_pass.get_frame(), 0);
		// Set the uniform, 0
		glUniform1i(effects["dof"].get_uniform_location("tex"), 0);
		// Sharp texture is taken from first pass
		// bind first pass, 1
		renderer::bind(first_pass.get_frame(), 1);
		//set sharp tex uniform, 1
		glUniform1i(effects["dof"].get_uniform_location("sharp"), 1);
		// Depth also taken from first pass
		// bind first pass **depth** to  TU 2
		renderer::bind(first_pass.get_depth(), 2);
		//set depth tex uniform, 2
		glUniform1i(effects["dof"].get_uniform_location("depth"), 2);
		// Set range and focus values
		// - range distance to chaser (get from camera)
		// - focus 0.07f
		glUniform1f(effects["dof"].get_uniform_location("range"), distance(ccam.get_position(), solar_objects["earth"].get_transform().position));
		glUniform1f(effects["dof"].get_uniform_location("focus"), 0.07f);
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