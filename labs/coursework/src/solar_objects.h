// solar_objects.h - Header file containing functions
// related to the solar objects in the scene
// Functions to load the objects, load the shadow plane,
// make the planets orbit the sun, shrink the sun and form
// a black hole
// Last modified - 24/03/2017

#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Load all solar objects
void load_solar_objects(map<string, mesh> &solar_objects, map<string, texture> &textures, map<string, texture> &normal_maps, map<string, float> &orbit_factors) {
	// SOLAR OBJECT MESHES
	solar_objects["sun"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["mercury"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["venus"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["earth"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["mars"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["clouds"] = mesh(geometry(geometry_builder::create_sphere(100, 100)));
	solar_objects["black_hole"] = mesh(geometry(geometry_builder::create_disk(20)));

	// TRANSFORM MESHES
	solar_objects["sun"].get_transform().scale = vec3(8.0f, 8.0f, 8.0f);
	solar_objects["sun"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["black_hole"].get_transform().position = vec3(0.0f, -1.0f, 0.0f);
	solar_objects["black_hole"].get_transform().rotate(vec3(0.5f, 0.0f, 0.0f));

	solar_objects["earth"].get_transform().translate(vec3(20.0f, 0.0f, -40.0f));
	solar_objects["earth"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));
	solar_objects["earth"].get_transform().rotate(vec3(0.0f, 0.0f, radians(23.44)));

	solar_objects["mercury"].get_transform().scale = 0.3f * solar_objects["venus"].get_transform().scale;
	solar_objects["mercury"].get_transform().translate(0.39f * solar_objects["earth"].get_transform().position);
	solar_objects["mercury"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["venus"].get_transform().scale = vec3(2.0f);
	solar_objects["venus"].get_transform().translate(0.72f * solar_objects["earth"].get_transform().position);
	solar_objects["venus"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["earth"].get_transform().scale = solar_objects["venus"].get_transform().scale;

	solar_objects["mars"].get_transform().scale = 0.53f * solar_objects["venus"].get_transform().scale;
	solar_objects["mars"].get_transform().translate(1.52f * solar_objects["earth"].get_transform().position);
	solar_objects["mars"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

	solar_objects["clouds"].get_transform().scale = vec3(1.01f) * solar_objects["earth"].get_transform().scale;
	solar_objects["clouds"].get_transform().position = solar_objects["earth"].get_transform().position;
	solar_objects["clouds"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

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

	solar_objects["sun"].get_material().set_emissive(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	solar_objects["sun"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	solar_objects["sun"].get_material().set_shininess(25.0f);

	solar_objects["black_hole"].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// OTHER PROPERTIES
	// Set orbit factors
	orbit_factors["mercury"] = 1.5f;
	orbit_factors["venus"] = 1.1f;
	orbit_factors["earth"] = 0.0f;
	orbit_factors["clouds"] = 0.0f;
	orbit_factors["mars"] = -0.235f;

	// LOAD TEXTURES
	// Solar objects
	textures["sunTex"] = texture("textures/sun_tex.jpg");
	textures["mercuryTex"] = texture("textures/mercury_tex.jpg");
	textures["venusTex"] = texture("textures/venus.jpg");
	textures["earthTex"] = texture("textures/earth_tex.jpg");
	textures["marsTex"] = texture("textures/mars.jpg");
	textures["cloudsTex"] = texture("textures/clouds.png");
	textures["black_holeTex"] = texture("textures/blackhole.jpg");

	// LOAD NORMAL MAPS
	normal_maps["sun"] = texture("textures/sun_normal_map.png");
	normal_maps["black_hole"] = texture("textures/black_hole_normal_map.png");
	normal_maps["mercury"] = texture("textures/mercury_normal_map.png");
	normal_maps["venus"] = texture("textures/venus_normal_map.png");
	normal_maps["earth"] = texture("textures/earth_normal_map.png");
	normal_maps["mars"] = texture("textures/mars_normal_map.png");
	normal_maps["clouds"] = texture("textures/clouds_normal_map.png");
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
	current_x = m.get_transform().position.x;
	current_z = m.get_transform().position.z;
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
	vec3 newPos(new_x, 0, new_z);
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

// Shrink sun and form a black hole
void black_hole(mesh &sun, mesh &black_hole, float delta_time)
{
	// If the sun hasn't disappeared yet, shrink it to 0
	if (sun.get_transform().scale != vec3(0.0f))
	{
		vec3 sun_size = sun.get_transform().scale;
		sun.get_transform().scale = max(sun_size - vec3(delta_time, delta_time, delta_time), 0.0f);
	}
	// Else, grow the black hole to scale 20
	else
	{
		vec3 black_hole_size = black_hole.get_transform().scale;
		black_hole.get_transform().scale = min(black_hole_size + vec3(delta_time, delta_time, delta_time), 20.0f);
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
			m.get_transform().rotate(vec3(0.0f, 0.0f, -delta_time / 2.0f));
		else if (e.first == "black_hole")
			m.get_transform().rotate(vec3(0.0f, -delta_time / 2.0f, 0.0f));
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