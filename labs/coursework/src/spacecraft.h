// spacecraft.h - Header file containing spacecraft functions
// Functions to load the Enterprise and Rama and control their
// motion
// Generate terrain creates terrain for use inside Rama
// Last modified - 17/04/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

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
		normalize(n);
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

// Load the Enterprise
void load_enterprise(array<mesh, 7> &enterprise, array<mesh, 2> &motions, map<string, texture> &textures, array<texture, 2> &motions_textures, map<string, texture> &normal_maps, map<string, effect> &effects)
{
	// ENTERPRISE MESHES
	// Saucer section
	enterprise[0] = mesh(geometry_builder::create_cylinder(100, 100));
	// Connection
	enterprise[1] = mesh(geometry_builder::create_box(vec3(1.0f, 4.0f, 1.0f)));
	// Shaft?
	enterprise[2] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.5f, 8.0f, 1.5f)));
	// Nacelles
	enterprise[3] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.0f, 10.0, 1.0f)));
	enterprise[4] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.0f, 10.0, 1.0f)));
	enterprise[5] = mesh(geometry_builder::create_box(vec3(0.1f, 5.0f, 1.0f)));
	enterprise[6] = mesh(geometry_builder::create_box(vec3(0.1f, 5.0f, 1.0f)));
	// Nacelle light domes
	motions[0] = mesh(geometry_builder::create_sphere(20, 20, vec3(0.5f)));
	motions[1] = mesh(geometry_builder::create_sphere(20, 20, vec3(0.5f)));

	// TRANSFORM MESHES
	// Enterprise (transform hierarchy in place)
	// Saucer section
	enterprise[0].get_transform().position = vec3(50.0f, 10.0f, 50.0f);
	enterprise[0].get_transform().scale = vec3(10.0f, 1.0f, 10.0f);
	// Connection
	enterprise[1].get_transform().scale = vec3(0.1f, 1.0f, 0.1f);
	enterprise[1].get_transform().position = vec3(0.0f, -2.5f, -0.45f);
	// Shaft?
	enterprise[2].get_transform().position = vec3(0.0f, -2.0f, -3.25f);
	enterprise[2].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	// Nacelles
	enterprise[3].get_transform().position = vec3(3.0f, -4.0f, -5.0f);
	enterprise[4].get_transform().position = vec3(-6.0f, 0.0f, 0.0f);
	// Nacelle connectors
	enterprise[5].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));
	enterprise[5].get_transform().position = vec3(1.5f, 1.5f, 2.5f);
	enterprise[5].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>() / 3.0f));
	enterprise[6].get_transform().rotate(vec3(0.0f, 0.0f, -pi<float>() / 3.0f));
	enterprise[6].get_transform().position = vec3(2.5f, -1.5f, 0.0f);
	// Nacelle light domes
	motions[0].get_transform().position = enterprise[0].get_transform().position - vec3(3.0f, -0.5f, 6.75f);
	motions[1].get_transform().position = enterprise[0].get_transform().position - vec3(-3.0f, -0.5f, 6.75f);

	// SET MATERIALS
	for (int i = 0; i < enterprise.size(); i++)
	{
		enterprise[i].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
		enterprise[i].get_material().set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
		enterprise[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
		enterprise[i].get_material().set_shininess(25.0f);
	}
	for (int i = 0; i < motions.size(); i++)
	{
		motions[i].get_material().set_emissive(vec4(1.0f, 0.0f, 0.0f, 1.0f));
		motions[i].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
		motions[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
		motions[i].get_material().set_shininess(25.0f);
	}

	// LOAD TEXTURES
	// Enterprise
	textures["enterprise"] = texture("textures/saucer.jpg");
	motions_textures[0] = texture("textures/white.jpg");
	motions_textures[1] = texture("textures/white.jpg");
	// LOAD NORMAL MAPS
	normal_maps["saucer"] = texture("textures/saucer_normal_map.png");

	// SHADERS
	// Load in shaders for enterprise
	effects["ship_eff"].add_shader("shaders/enterprise.vert", GL_VERTEX_SHADER);
	vector<string> ship_eff_frag_shaders{ "shaders/enterprise.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	effects["ship_eff"].add_shader(ship_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["ship_eff"].build();
}

// Load Rama
void load_rama(mesh &rama, array<mesh, 6> &rama_terrain, map<string, texture> &textures, array<texture, 4> &terrain_texs, map<string, texture> &normal_maps, map<string, effect> &effects)
{
	//RAMA
	rama = mesh(geometry_builder::create_cylinder(100, 100));

	// TRANSFORM
	rama.get_transform().scale = vec3(20.0f, 40.0f, 20.0f);
	rama.get_transform().position = vec3(70.0f, 0.0f, 70.0f);
	rama.get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()));

	// SET MATERIAL
	rama.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	rama.get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	rama.get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	rama.get_material().set_shininess(25.0f);

	// LOAD TEXTURES
	textures["ramaOutTex"] = texture("textures/brick.jpg");
	textures["ramaInTex"] = texture("textures/Earth_tex.tga");
	textures["ramaGrassTex"] = texture("textures/grass.jpg");
	textures["blend_map"] = texture("textures/blend_map.jpg");

	// LOAD NORMAL MAP
	normal_maps["ramaOut"] = texture("textures/brick_normalmap.jpg");

	// CREATE TERRAIN
	// Geometry to load into
	geometry geom;
	// Load height map
	texture height_map("textures/heightmap.jpg");
	// Generate terrain
	generate_terrain(geom, height_map, 10, 10, 2.0f);
	// Use geometry to create terrain mesh
	rama_terrain[0] = mesh(geom);
	rama_terrain[1] = mesh(geom);
	rama_terrain[2] = mesh(geom);
	rama_terrain[3] = mesh(geom);
	rama_terrain[4] = mesh(geom);
	rama_terrain[5] = mesh(geom);
	// Transform terrain (hierarchy in place)
	rama_terrain[0].get_transform().position = rama.get_transform().position;
	rama_terrain[0].get_transform().scale = vec3(4.0f, 1.0f, 1.0f);
	rama_terrain[0].get_transform().translate(vec3(0.0f, -8.66f, 0.0f));
	rama_terrain[1].get_transform().rotate(vec3(pi<float>() / 3.0f, 0.0f, 0.0f));
	rama_terrain[1].get_transform().translate(vec3(0.0f, 4.25f, -7.5f));
	rama_terrain[2].get_transform().rotate(vec3(pi<float>() / 3.0f, 0.0f, 0.0f));
	rama_terrain[2].get_transform().translate(vec3(0.0f, 4.25f, -7.5f));
	rama_terrain[3].get_transform().rotate(vec3(pi<float>() / 3.0f, 0.0f, 0.0f));
	rama_terrain[3].get_transform().translate(vec3(0.0f, 4.25f, -7.5f));
	rama_terrain[4].get_transform().rotate(vec3(pi<float>() / 3.0f, 0.0f, 0.0f));
	rama_terrain[4].get_transform().translate(vec3(0.0f, 4.25f, -7.5f));
	rama_terrain[5].get_transform().rotate(vec3(pi<float>() / 3.0f, 0.0f, 0.0f));
	rama_terrain[5].get_transform().translate(vec3(0.0f, 4.25f, -7.5f));
	// Set materials
	for (auto &m : rama_terrain)
	{
		m.get_material().set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1.0f));
		m.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
		m.get_material().set_shininess(20.0f);
		m.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	// Load textures
	terrain_texs[0] = texture("textures/sand.jpg");
	terrain_texs[1] = texture("textures/grass.jpg");
	terrain_texs[2] = texture("textures/stone.jpg");
	terrain_texs[3] = texture("textures/snow.jpg");

	// SHADERS
	// Load in shaders for rama
	effects["inside_eff"].add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	vector<string> inside_eff_frag_shaders{ "shaders/inside.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_fog.frag" };
	effects["inside_eff"].add_shader(inside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["inside_eff"].build();

	effects["terrain_eff"].add_shader("shaders/terrain.vert", GL_VERTEX_SHADER);
	vector<string> terrain_eff_frag_shaders{ "shaders/terrain.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_weighted_texture_4.frag", "shaders/part_fog.frag" };
	effects["terrain_eff"].add_shader(terrain_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["terrain_eff"].build();

	effects["outside_eff"].add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> outside_eff_frag_shaders{ "shaders/blend.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	effects["outside_eff"].add_shader(outside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	effects["outside_eff"].build();
}

// User controlled motion of Enterprise
void move_enterprise(array<mesh, 7> &enterprise, array<mesh, 2> &motions, vec3 engage, float delta_time)
{
	enterprise[0].get_transform().translate(engage);
	motions[0].get_transform().translate(engage);
	motions[1].get_transform().translate(engage);
	// Spin nacelle domes
	motions[0].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[1].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
}

// Define Rama terrain orbit
void orbit(mesh &m, vec3 rotCenter, float delta_time)
{
	// Get centre of orbit, current position of orbiting
	// body and it's radius
	float current_y, current_z, rotAngle, radius;
	current_y = m.get_transform().position.y - rotCenter.y;
	current_z = m.get_transform().position.z - rotCenter.z;
	radius = distance(m.get_transform().position, rotCenter);
	// Calculate orbit angle, correctint for quadrants of xz axes
	if (current_y < 0)
	{
		if (current_z < 0)
			rotAngle = atan(current_z / current_y) - radians(180.0f);
		else
			rotAngle = atan(current_z / current_y) + radians(180.0f);
	}
	else
		rotAngle = atan(current_z / current_y);
	// Increment rotAngle to calcualte next position taking into
	// account the planets unique orbit speed
	rotAngle = radians(rotAngle * (180.0f / pi<float>()) + 0.1f + delta_time);
	// Correct for full rotation
	if (rotAngle > radians(360.0f))
		rotAngle = 0.0f;
	// Calculate new position
	float new_y = rotCenter.y + (radius * cosf(rotAngle));
	float new_z = rotCenter.z + (radius * sinf(rotAngle));
	vec3 newPos(rotCenter.x, new_y, new_z);
	m.get_transform().position = newPos;
	// Spin surface
	m.get_transform().rotate(vec3(radians(0.1f + delta_time), 0.0f, 0.0f));
}

// Rotate Rama
void spin_rama(mesh &rama, mesh &terrain, float delta_time)
{
	// Rotate the outside
	rama.get_transform().rotate(vec3(0.0f, -radians(0.1f + delta_time), 0.0f));
	// Rotate the terrain inside (only need to rotate the first terrain
	// object, the rest will follow thanks to the transform hierarchy)
	orbit(terrain, rama.get_transform().position, delta_time);
}