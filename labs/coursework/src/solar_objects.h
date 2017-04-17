// solar_objects.h - Header file containing functions
// related to the solar objects in the scene
// Functions to load the objects, load the shadow plane,
// make the planets orbit the sun, shrink the sun and form
// a black hole
// Last modified - 17/04/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

default_random_engine generator;
uniform_int_distribution<int> distribution(1, 10);
uniform_real_distribution<float> distribution_y(-8, 8);
uniform_real_distribution<float> distribution_theta(0, 2 * pi<float>());

// Load all solar objects
void load_solar_objects(map<string, mesh> &solar_objects, geometry &distortion, map<string, texture> &textures, array<texture, 14> &jupiter_texs, map<string, texture> &normal_maps, map<string, float> &orbit_factors, map<string, effect> &effects) {
	vector<vec3> distortion_position;
	distortion_position.push_back(vec3(0.0f, 0.0f, 0.0f));
	// Create geometry using these points
	distortion.add_buffer(distortion_position, BUFFER_INDEXES::POSITION_BUFFER);
	// Set geometry type to points
	distortion.set_type(GL_POINTS);

	// SOLAR OBJECT MESHES 
	solar_objects["sun"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["mercury"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["venus"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["earth"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["mars"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["jupiter"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["clouds"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["black_hole"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["comet"] = mesh(geometry("models/Asteroid.obj"));
	
	// TRANSFORM MESHES
	solar_objects["earth"].get_transform().translate(vec3(20.0f, 0.0f, -40.0f));
	solar_objects["earth"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));
	solar_objects["earth"].get_transform().rotate(vec3(0.0f, 0.0f, radians(23.44)));
	solar_objects["earth"].get_transform().scale = vec3(2.0f);

	vec3 earth_scale = solar_objects["earth"].get_transform().scale;
	vec3 earth_position = solar_objects["earth"].get_transform().position;

	solar_objects["sun"].get_transform().scale = vec3(8.0f, 8.0f, 8.0f);
	solar_objects["sun"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["mercury"].get_transform().scale = 0.3f * earth_scale;
	solar_objects["mercury"].get_transform().translate(0.39f * earth_position);
	solar_objects["mercury"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["venus"].get_transform().scale = 0.8f * earth_scale;
	solar_objects["venus"].get_transform().translate(0.72f * solar_objects["earth"].get_transform().position);
	solar_objects["venus"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["mars"].get_transform().scale = 0.53f * earth_scale;
	solar_objects["mars"].get_transform().translate(1.52f * earth_position);
	solar_objects["mars"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["jupiter"].get_transform().scale = 11.2f * earth_scale;
	solar_objects["jupiter"].get_transform().translate(5.2f * earth_position);
	solar_objects["jupiter"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["clouds"].get_transform().scale = vec3(1.01f) * earth_scale;
	solar_objects["clouds"].get_transform().position = earth_position;
	solar_objects["clouds"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["comet"].get_transform().position = vec3(0.0f, 0.0f, 50.0f);
	solar_objects["comet"].get_transform().scale = vec3(0.1f);

	// SET MATERIALS
	solar_objects["earth"].get_material().set_specular(vec4(1.0f, 0.65f, 0.0f, 1.0f));
	solar_objects["earth"].get_material().set_shininess(25.0f);

	solar_objects["clouds"].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	solar_objects["clouds"].get_material().set_shininess(0.0f);

	solar_objects["venus"].get_material().set_specular(vec4(0.25f, 0.1625f, 0.0f, 1.0f));
	solar_objects["venus"].get_material().set_shininess(5.0f);

	solar_objects["mars"].get_material().set_specular(vec4(0.5f, 0.325f, 0.0f, 1.0f));
	solar_objects["mars"].get_material().set_shininess(2.0f);

	solar_objects["mercury"].get_material().set_specular(vec4(0.256777, 0.137622, 0.086014, 1.0f));
	solar_objects["mercury"].get_material().set_shininess(12.8f);

	solar_objects["jupiter"].get_material().set_specular(vec4(0.25f, 0.1625f, 0.0f, 1.0f));
	solar_objects["jupiter"].get_material().set_shininess(2.0f);

	solar_objects["sun"].get_material().set_emissive(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	solar_objects["sun"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	solar_objects["sun"].get_material().set_shininess(25.0f);

	// OTHER PROPERTIES
	// Set orbit factors
	orbit_factors["mercury"] = 1.5f;
	orbit_factors["venus"] = 1.1f;
	orbit_factors["earth"] = 0.0f;
	orbit_factors["clouds"] = 0.0f;
	orbit_factors["mars"] = -0.235f;
	orbit_factors["jupiter"] = -0.4f;
	
	// LOAD TEXTURES
	// Solar objects
	textures["sunTex"] = texture("textures/sun_tex.jpg");
	textures["mercuryTex"] = texture("textures/mercury_tex.jpg");
	textures["venusTex"] = texture("textures/venus.jpg");
	textures["earthTex"] = texture("textures/earth_tex.jpg");
	textures["marsTex"] = texture("textures/mars.jpg");
	textures["cloudsTex"] = texture("textures/clouds.png");
	textures["black_holeTex"] = texture("textures/blackhole.jpg");
	textures["cometTex"] = texture("textures/asteroid.jpg");

	jupiter_texs[0] = texture("textures/jupiter1.gif");
	jupiter_texs[1] = texture("textures/jupiter2.gif");
	jupiter_texs[2] = texture("textures/jupiter3.gif");
	jupiter_texs[3] = texture("textures/jupiter4.gif");
	jupiter_texs[4] = texture("textures/jupiter5.gif");
	jupiter_texs[5] = texture("textures/jupiter6.gif");
	jupiter_texs[6] = texture("textures/jupiter7.gif");
	jupiter_texs[7] = texture("textures/jupiter8.gif");
	jupiter_texs[8] = texture("textures/jupiter9.gif");
	jupiter_texs[9] = texture("textures/jupiter10.gif");
	jupiter_texs[10] = texture("textures/jupiter11.gif");
	jupiter_texs[11] = texture("textures/jupiter12.gif");
	jupiter_texs[12] = texture("textures/jupiter13.gif");
	jupiter_texs[13] = texture("textures/jupiter14.gif");

	// LOAD NORMAL MAPS
	normal_maps["sun"] = texture("textures/sun_normal_map.png");
	normal_maps["black_hole"] = texture("textures/black_hole_normal_map.png");
	normal_maps["mercury"] = texture("textures/mercury_normal_map.png");
	normal_maps["venus"] = texture("textures/venus_normal_map.png");
	normal_maps["earth"] = texture("textures/earth_normal_map.png");
	normal_maps["mars"] = texture("textures/mars_normal_map.png");
	normal_maps["clouds"] = texture("textures/clouds_normal_map.png");
	normal_maps["comet"] = texture("textures/asteroid_normal_map.png");

	// SHADERS
	// Load in shaders for planets
	effects["planet_eff"].add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> planet_eff_frag_shaders{ "shaders/simple_texture.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_spot.frag", "shaders/part_fog.frag" };
	effects["planet_eff"].add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["planet_eff"].build();

	// Load in shaders for clouds
	effects["cloud_eff"].add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> cloud_eff_frag_shaders{ "shaders/cloud_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_fog.frag" };
	effects["cloud_eff"].add_shader(cloud_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["cloud_eff"].build();

	// Load in shaders for sun
	effects["sun_eff"].add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	effects["sun_eff"].add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["sun_eff"].add_shader("shaders/explode.geom", GL_GEOMETRY_SHADER);
	effects["sun_eff"].build();

	// Load in distortion shaders
	effects["distortion_eff"].add_shader("shaders/shader.vert", GL_VERTEX_SHADER);
	effects["distortion_eff"].add_shader("shaders/billboard.geom", GL_GEOMETRY_SHADER);
	effects["distortion_eff"].add_shader("shaders/shader.frag", GL_FRAGMENT_SHADER);
	effects["distortion_eff"].build();

	// Load in shaders for weather changes
	effects["weather_eff"].add_shader("shaders/weather.vert", GL_VERTEX_SHADER);
	vector<string> weather_eff_frag_shaders{ "shaders/weather.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag" };
	effects["weather_eff"].add_shader(weather_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["weather_eff"].build();
}

// Load the shadow plane
void load_shadow_plane(mesh &shadow_plane, map<string, texture> &textures, map<string, texture> &normal_maps)
{
	// DEMO SHADOW PLANE
	shadow_plane = mesh(geometry_builder::create_plane());
	
	// TRANSFORM MESH
	shadow_plane.get_transform().position = vec3(40.0f, -15.0f, 0.0f);
	shadow_plane.get_transform().scale = vec3(0.5f, 1.0f, 1.5f);

	// SET MATERIAL
	shadow_plane.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	shadow_plane.get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shadow_plane.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	shadow_plane.get_material().set_shininess(25.0f);

	// LOAD TEXTURE
	textures["planeTex"] = texture("textures/white.jpg");

	// LOAD NORMAL MAP
	normal_maps["plane"] = texture("textures/plane_normal_map.png");
}

// Define planetary orbits
void orbit(mesh &m, mesh &sun, float orbit_factor, bool destroy_solar_system, float delta_time)
{
	// Get centre of orbit, current position of orbiting
	// body and it's radius
	vec3 rotCenter = sun.get_transform().position;
	float current_x, current_z, rotAngle, radius;
	current_x = m.get_transform().position.x - rotCenter.x;
	current_z = m.get_transform().position.z - rotCenter.y;
	radius = distance(m.get_transform().position, rotCenter);
	// If planet has fallen into black hole, leave it there
	// and shrink to zero
	if (radius < 0.3f)
	{
		m.get_transform().scale = vec3(0.0f);
		return;
	}
	// Calculate orbit angle, correctint for quadrants of xz axes
	if (current_x < 0)
	{
		if (current_z < 0)
			rotAngle = atan(current_z / current_x) - radians(180.0f);
		else
			rotAngle = atan(current_z / current_x) + radians(180.0f);
	}
	else
		rotAngle = atan(current_z / current_x);
	// Increment rotAngle to calcualte next position taking into
	// account the planets unique orbit speed
	rotAngle = radians(rotAngle * (180.0f / pi<float>()) + 0.5f + orbit_factor);
	// Correct for full rotation
	if (rotAngle > radians(360.0f))
		rotAngle = 0.0f;
	// Calculate new position
	float new_x = rotCenter.x + (radius * cosf(rotAngle));
	float new_z = rotCenter.z + (radius * sinf(rotAngle));
	vec3 newPos(new_x, rotCenter.y, new_z);
	m.get_transform().position = newPos;
	// Spin planet
	m.get_transform().rotate(vec3(0.0f, 0.0f, 2 * delta_time));
	// If the black hole has formed then move the planet closer
	// and shrink
	if (destroy_solar_system == true && sun.get_transform().scale == vec3(0.0f))
	{
		float factor = 1.0f / (radius * radius);
		m.get_transform().translate(-factor * m.get_transform().position);
		m.get_transform().scale -= (factor * m.get_transform().scale);
	}
}

// Define asteroid orbit
void asteroid_orbit(mesh &m, mesh &sun, float orbit_factor, bool destroy_solar_system, float delta_time)
{
	m.get_transform().translate(vec3(2.0f * delta_time, 0.0f, 0.0f));
	m.get_transform().rotate(vec3(-half_pi<float>() * delta_time, 0.0f, 0.0f));
}

// Shrink sun and form a black hole
void black_hole(mesh &sun, mesh &black_hole, float &distortion_size, float &blur_factor, float delta_time)
{
	// If the sun hasn't disappeared yet, shrink it to 0
	if (sun.get_transform().scale != vec3(0.0f))
	{
		vec3 sun_size = sun.get_transform().scale;
		sun.get_transform().scale = max(sun_size - vec3(delta_time, delta_time, delta_time), 0.0f);
	}
	// Else, grow the black hole to scale 20 and make the scene
	// more blurry as it grows
	else
	{
		blur_factor = std::max(blur_factor - 0.001f, 0.3f);
		vec3 black_hole_size = black_hole.get_transform().scale;
		black_hole.get_transform().scale = min(black_hole_size + vec3(delta_time, delta_time, delta_time), 20.0f);
		distortion_size = std::min(distortion_size + 2.5f * delta_time, 45.0f);
	}
}

// Control motion of all solar objects
void system_motion(map<string, mesh> &solar_objects, map<string, float> orbit_factors, 
				   bool destroy_solar_system, float delta_time)
{
	// All planets orbit sun
	auto sun = solar_objects["sun"];
	// Control orbiting objects motion
	for (auto &e : solar_objects)
	{
		auto &m = e.second;
		// The sun and the black hole spin around the origin
		if (e.first == "sun")
		{
			m.get_transform().rotate(vec3(0.0f, 0.0f, -delta_time / 2.0f));
		}
		else if (e.first == "black_hole")
			m.get_transform().rotate(vec3(0.0f, -delta_time / 2.0f, 0.0f));
		else if (e.first == "comet")
		{
			asteroid_orbit(m, sun, orbit_factors[e.first], destroy_solar_system, delta_time);
		}
		// The clouds rotate faster than the Earth
		else if (e.first == "clouds")
		{
			m.get_transform().rotate(vec3(0.0f, 0.0f, 2.0f * delta_time));
			orbit(m, sun, orbit_factors[e.first], destroy_solar_system, delta_time);
		}
		// All planets simply orbit the sun
		else
		{
			orbit(m, sun, orbit_factors[e.first], destroy_solar_system, delta_time);
		}
	}
}

vec3 generate_random_sphere_point(float radius)
{
	float y = distribution_y(generator);
	float theta = distribution_theta(generator);
	float lat = asinf(y / radius);
	float x = radius * cosf(lat)*cosf(theta);
	float z = radius * cosf(lat)*sinf(theta);
	return vec3(x, y, z);
}

void solar_activity(float &random, float &peak_factor, float &explode_factor, vec3 &sun_activity, float total_time)
{
	// If random number between 1 and 10 is greater than 2
	// then increase the peak_factor, else decrease it till
	// it reaches it's minimum of 2.5
	if (random > 2.0f)
		peak_factor = std::min(peak_factor + 0.01f, 5.0f);
	else
		peak_factor = std::max(peak_factor - 0.01f, 2.5f);

	// Update the explode_factor - base on sin wave
	explode_factor = std::max(0.25f, abs(0.0f + sinf(total_time)));

	// Only update random when total_time modulo 10 is less than 0.1
	// This gives time for the change in explode factor to be noticed
	if (mod(total_time, 10.0f) <= 0.1f)
		random = distribution(generator);

	if (mod(total_time, 20.0f) <= 0.1f)
	{
		sun_activity = generate_random_sphere_point(8.0f);
		peak_factor = 0.0f;
	}
}