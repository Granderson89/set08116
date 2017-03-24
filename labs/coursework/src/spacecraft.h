#pragma once

#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

void load_enterprise(array<mesh, 7> &enterprise, array<mesh, 2> &motions, map<string, texture> &textures, array<texture, 2> &motions_textures, map<string, texture> &normal_maps)
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
}

void load_rama(mesh &rama, map<string, texture> &textures, map<string, texture> &normal_maps)
{
	//RAMA
	rama = mesh(geometry_builder::create_cylinder(100, 100));

	// TRANSFORM
	rama.get_transform().scale = vec3(10.0f, 20.0f, 10.0f);
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
}

void move_enterprise(array<mesh, 7> &enterprise, array<mesh, 2> &motions, vec3 engage, float delta_time)
{
	enterprise[0].get_transform().translate(engage);
	motions[0].get_transform().translate(engage);
	motions[1].get_transform().translate(engage);
	// Spin nacelle domes
	motions[0].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[1].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
}

void spin_rama(mesh &rama, float delta_time)
{
	rama.get_transform().rotate(vec3(0.0f, delta_time, 0.0f));
}