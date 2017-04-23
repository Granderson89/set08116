// solar_objects.h - Header file containing functions
// related to the solar objects in the scene
// Functions to load the objects, load the shadow plane,
// make the planets orbit the sun, shrink the sun and form
// a black hole
// Last modified - 20/04/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Random generator for sun activity
default_random_engine generator;
uniform_int_distribution<int> distribution(1, 10);
uniform_real_distribution<float> distribution_y(-8, 8);
uniform_real_distribution<float> distribution_theta(0, 2 * pi<float>());

// Function for creating terrain geometry from a height map
void generate_terrain(geometry &geom, const texture &height_map, unsigned int width, unsigned int depth,
	float height_scale) {
	// Contains our position data
	vector<vec3> positions;
	// Contains our normal data
	vector<vec3> normals;
	// Contains our texture coordinate data
	vector<vec2> tex_coords;
	// Contains our texture weights
	vector<vec4> tex_weights;
	// Contains our index data
	vector<unsigned int> indices;

	// Extract the texture data from the image
	glBindTexture(GL_TEXTURE_2D, height_map.get_id());
	auto data = new vec4[height_map.get_width() * height_map.get_height()];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void *)data);

	// Determine ratio of height map to geometry
	float width_point = static_cast<float>(width) / static_cast<float>(height_map.get_width());
	float depth_point = static_cast<float>(depth) / static_cast<float>(height_map.get_height());

	// Point to work on
	vec3 point;

	// Part 1 - Iterate through each point, calculate vertex and add to vector
	for (int x = 0; x < height_map.get_width(); ++x) {
		// Calculate x position of point
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (int z = 0; z < height_map.get_height(); ++z) {
			// *********************************
			// Calculate z position of point
			point.z = -(depth / 2.0f) + (depth_point * static_cast<float>(z));
			// *********************************
			// Y position based on red component of height map data
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			// Add point to position data
			positions.push_back(point);
		}
	}

	// Part 1 - Add index data
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x) {
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y) {
			// Get four corners of patch
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			// *********************************
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_height()) + x + 1;
			// *********************************
			// Push back indices for triangle 1 (tl,br,bl)
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			// Push back indices for triangle 2 (tl,tr,br)
			// *********************************
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
			// *********************************
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());

	// Part 2 - Calculate normals for the height map
	for (unsigned int i = 0; i < indices.size() / 3; ++i) {
		// Get indices for the triangle
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];

		// Calculate two sides of the triangle
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];

		// Normal is normal(cross product) of these two sides
		// *********************************
		vec3 n = normalize(side2 * side1);
		// Add to normals in the normal buffer using the indices for the triangle
		normals[idx1] = normals[idx1] + n;
		normals[idx2] = normals[idx2] + n;
		normals[idx3] = normals[idx3] + n;
		// *********************************
	}

	// Normalize all the normals
	for (auto &n : normals) {
		// *********************************
		n = normalize(n);
		// *********************************
	}

	// Part 3 - Add texture coordinates for geometry
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			tex_coords.push_back(vec2(width_point * x, depth_point * z));
		}
	}

	// Part 4 - Calculate texture weights for each vertex
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			// Calculate tex weight
			vec4 tex_weight(clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.0f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.15f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.5f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.9f) / 0.25f, 0.0f, 1.0f));

			// *********************************
			// Sum the components of the vector
			float total = tex_weight.x + tex_weight.y + tex_weight.z + tex_weight.w;
			// Divide weight by sum
			tex_weight = tex_weight / total;
			// Add tex weight to weights
			tex_weights.push_back(tex_weight);
			// *********************************
		}
	}

	// Add necessary buffers to the geometry
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_buffer(tex_weights, BUFFER_INDEXES::TEXTURE_COORDS_1);
	geom.add_index_buffer(indices);

	// Delete data
	delete[] data;
}

// Load all solar objects
void load_solar_objects(map<string, mesh> &solar_objects, geometry &distortion, map<string, 
	texture> &textures, array<texture, 14> &jupiter_texs, map<string, texture> &normal_maps, 
	map<string, float> &orbit_factors, 
	map<string, effect> &effects) 
{
	// BLACK HOLE DISTORTION
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

	solar_objects["comet"].get_transform().position = vec3(-50.0f, 0.0f, 50.0f);
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
	effects["sun_eff"].add_shader("shaders/dynamic_sun.geom", GL_GEOMETRY_SHADER);
	effects["sun_eff"].build();

	// Load in distortion shaders
	effects["distortion_eff"].add_shader("shaders/shader.vert", GL_VERTEX_SHADER);
	effects["distortion_eff"].add_shader("shaders/billboard_env.geom", GL_GEOMETRY_SHADER);
	effects["distortion_eff"].add_shader("shaders/shader.frag", GL_FRAGMENT_SHADER);
	effects["distortion_eff"].build();

	// Load in shaders for weather changes
	effects["weather_eff"].add_shader("shaders/weather.vert", GL_VERTEX_SHADER);
	vector<string> weather_eff_frag_shaders{ "shaders/weather.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag" };
	effects["weather_eff"].add_shader(weather_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["weather_eff"].build();
}

// Load the terrain
void load_terrain(mesh &terrain, mesh &cube, array<texture, 4> &terrain_texs, map<string, effect> &effects)
{
	cube = mesh(geometry(geometry_builder::create_box(vec3(50.0f, 20.0f, 100.0f))));

	// CREATE TERRAIN
	// Geometry to load into
	geometry geom;
	// Load height map
	texture height_map("textures/heightmap.jpg");
	// Generate terrain
	generate_terrain(geom, height_map, 50, 100, 10.0f);
	// Use geometry to create terrain mesh
	terrain = mesh(geom);
	// Transform terrain
	terrain.get_transform().position = vec3(50.0f, -10.0f, -10.0f);
	cube.get_transform().position = terrain.get_transform().position + vec3(0.0f, 5.0f, 0.0f);
	// Set material
	terrain.get_material().set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	terrain.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	terrain.get_material().set_shininess(20.0f);
	terrain.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// Load textures
	terrain_texs[0] = texture("textures/sand.jpg");
	terrain_texs[1] = texture("textures/grass.jpg");
	terrain_texs[2] = texture("textures/stone.jpg");
	terrain_texs[3] = texture("textures/snow.jpg");

	// SHADER
	effects["terrain_eff"].add_shader("shaders/terrain.vert", GL_VERTEX_SHADER);
	vector<string> terrain_eff_frag_shaders{ "shaders/terrain.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_weighted_texture_4.frag", "shaders/part_fog.frag", "shaders/part_shadow.frag" };
	effects["terrain_eff"].add_shader(terrain_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["terrain_eff"].build();
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

// Define asteroid motion
void asteroid_motion(mesh &m, mesh &sun, float orbit_factor, bool destroy_solar_system, float delta_time)
{
	// Simply move the asteroid along the x axis and spin
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
		{
			m.get_transform().rotate(vec3(0.0f, -delta_time / 2.0f, 0.0f));
		}
		else if (e.first == "comet")
		{
			asteroid_motion(m, sun, orbit_factors[e.first], destroy_solar_system, delta_time);
		}
		// The clouds rotate faster than the Earth
		else if (e.first == "clouds")
		{
			m.get_transform().rotate(vec3(0.0f, 0.0f, 2.0f * delta_time));
			orbit(m, sun, orbit_factors[e.first], destroy_solar_system, delta_time);
		}
		// All else simply orbit the sun
		else
		{
			orbit(m, sun, orbit_factors[e.first], destroy_solar_system, delta_time);
		}
	}
}

// Generate a random point on a sphere of given radius
// Used to randomise area of activity on the sun
vec3 generate_random_sphere_point(float radius)
{
	// Get a random y
	float y = distribution_y(generator);
	// Get a random angle
	float theta = distribution_theta(generator);
	// Calcuate lattitude for the y
	float lat = asinf(y / radius);
	// Calculate x and z
	float x = radius * cosf(lat)*cosf(theta);
	float z = radius * cosf(lat)*sinf(theta);
	// Return the calculated coordinate
	return vec3(x, y, z);
}

// Makes the sun dynamic
// explode_factor makes the entire sun's surface look like it's moving
// peak_factor is used to amplify one small area of the surface to 
// look like a flare
// sun_activity is a point on the surface which may be amplified depending
// on peak_factor. This point is updated periodically
void solar_activity(float &random, float &peak_factor, float &explode_factor, vec3 &sun_activity, float total_time)
{
	// If random number between 1 and 10 is greater than 2
	// then increase the peak_factor till it reaches its
	// maximum of 5.0, else decrease it till it reaches 
	// it's minimum of 2.5
	// Makes a smooth increase in the flare
	if (random > 2.0f)
		peak_factor = std::min(peak_factor + 0.01f, 5.0f);
	else
		peak_factor = std::max(peak_factor - 0.01f, 2.5f);

	// Update the explode_factor - base on sin wave (minimum of 0.25)
	explode_factor = std::max(0.25f, abs(0.0f + sinf(total_time)));

	// Only update random when total_time modulo 10 is less than 0.1
	// This gives time for the change in explode factor to be noticed
	if (mod(total_time, 10.0f) <= 0.1f)
		random = distribution(generator);

	// Change the active area on the surface periodically and reset the
	// peak factor
	if (mod(total_time, 20.0f) <= 0.1f)
	{
		sun_activity = generate_random_sphere_point(8.0f);
		peak_factor = 0.0f;
	}
}