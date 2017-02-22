#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
geometry stars;
map<string, mesh> meshes;
effect eff;
effect sbeff;
effect ceff;
map<string, texture> textures;
texture blend_map;
texture tex;
target_camera tcam;
point_light light;
double cursor_x = 0.0;
double cursor_y = 0.0;
cubemap cube_map;
float rotAngle = 0.0f;
bool destroy_solar_system = false;

void orbit(mesh &m, mesh &sun, string name, float delta_time)
{
	vec3 rotCenter = sun.get_transform().position;
	float current_x, current_z, rotAngle, radius;
	current_x = m.get_transform().position.x;
	current_z = m.get_transform().position.z;
	radius = distance(m.get_transform().position, rotCenter);
	// If planet has fallen into black hole, leave it there
	if (radius < 0.2f)
	{
		return;
	}
	// Correct for quadrants of xz axes
	if (current_x < 0)
	{
		if (current_z < 0)
		{
			rotAngle = atan(current_z / current_x) - radians(180.0f);
		}
		else

		{
			rotAngle = atan(current_z / current_x) + radians(180.0f);
		}
	}
	else
	{
		rotAngle = atan(current_z / current_x);
	}
	// Increment rotAngle to calcualte next position
	rotAngle = rotAngle * (180.0f / pi<float>());
	rotAngle += 0.5f;
	rotAngle = radians(rotAngle);
	float factor = 1.0f / (radius * radius);
	cout << name << " " << radius << " " << rotAngle << endl;
	cout << "(" << m.get_transform().position.x << ", " << m.get_transform().position.z << endl;
	// Correct for full rotation
	if (rotAngle > radians(360.0f))
	{
		rotAngle = 0.0f;
	}
	// Calculate new position
	float new_x = rotCenter.x + (radius * cosf(rotAngle));
	float new_z = rotCenter.z + (radius * sinf(rotAngle));
	vec3 newPos = vec3(new_x, 0, new_z);
	m.get_transform().position = newPos;
	m.get_transform().rotate(vec3(0.0f, 2 * delta_time, 0.0f));
	if (destroy_solar_system == true && sun.get_transform().scale == vec3(0.0f))
	{
		m.get_transform().translate(-factor * m.get_transform().position);
		m.get_transform().scale -= (factor * m.get_transform().scale);
	}

}

void black_hole(float delta_time)
{
	if (meshes["sun"].get_transform().scale != vec3(0.0f))
	{
		auto &m = meshes["sun"];
		vec3 sun_size = m.get_transform().scale;
		m.get_transform().scale = max(sun_size - vec3(delta_time, delta_time, delta_time), 0.0f);
	}
	else
	{
		auto &m = meshes["black_hole"];
		vec3 black_hole_size = m.get_transform().scale;
		m.get_transform().scale = min(black_hole_size + vec3(delta_time, delta_time, delta_time), 20.0f);
	}
}

bool load_content() {
	// Create scene
	meshes["earth"] = mesh(geometry("models/Earth.obj"));
	meshes["clouds"] = mesh(geometry("models/Earth.obj"));
	meshes["sun"] = mesh(geometry("models/Earth.obj"));
	meshes["black_hole"] = mesh(geometry(geometry_builder::create_disk(20)));
	stars = geometry_builder::create_box();

	// Transform objects
	meshes["earth"].get_transform().scale = vec3(1.0f);
	meshes["earth"].get_transform().translate(vec3(10.0f, 0.0f, -30.0f));
	meshes["earth"].get_transform().rotate(vec3(0.0f, radians(23.44), 0.0f));
	meshes["clouds"].get_transform().scale = vec3(1.005f, 1.005f, 1.005f);
	meshes["clouds"].get_transform().position = meshes["earth"].get_transform().position;
	meshes["sun"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["sun"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));
	meshes["black_hole"].get_transform().rotate(vec3(0.5f, 0.0f, 0.0f));

	// Set materials
	// - all emissive is black
	// - all specular is white
	// - all shininess is 25
	// Red box
	material mat;
	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 0.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["earth"].set_material(mat);

	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["sun"].set_material(mat);

	mat.set_diffuse(vec4(1.0f, 1.0f, 0.0f, 1.0f));
	meshes["black_hole"].set_material(mat);

	// Load textures
	textures["earthTex"] = texture("textures/Earth_tex.tga");
	textures["sunTex"] = texture("textures/sun_tex.jpg");
	textures["cloudsTex"] = texture("textures/clouds.png");
	textures["black_holeTex"] = texture("textures/blackhole.jpg");

	// Create background object
	string background = "textures/stars2.jpg";
	cube_map = cubemap({ background, background, background, background, background, background });
	
	// Set lighting values, Position (-25, 10, -10)
	light.move(vec3(5.0f, 0.0f, 0.0f));
	// Light colour white
	light.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// Set range to 20
	light.set_range(100.0f);
	// Load in shaders for planets
	eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	// Load in shaders for background
	sbeff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sbeff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	sbeff.build();

	// Load in shaders for clouds
	ceff.add_shader("shaders/cloud_texture.vert", GL_VERTEX_SHADER);
	ceff.add_shader("shaders/cloud_texture.frag", GL_FRAGMENT_SHADER);
	ceff.build();

	// Set camera properties
	tcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	// *********************************
	// Use this to focus on object of choice
	// cam.set_target(meshes[].get_transform().position);
	// *********************************
	tcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	tcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;
}

bool update(float delta_time) {
	// Check if solar system is to be destroyed
	if (destroy_solar_system == true)
	{
		black_hole(delta_time);
	}
	// All planets orbit "sun"
	auto sun = meshes["sun"];
	// Control solar objects motion
	for (auto &e : meshes)
	{
		auto &m = e.second;
		// The sun and the black hole rotate around the origin
		if (e.first == "sun" || e.first == "black_hole")
		{
			m.get_transform().rotate(vec3(0.0f, delta_time / 2.0f, 0.0f));
		}
		// The clouds rotate twice as fast as the Earth
		else if (e.first == "clouds")
		{
			m.get_transform().rotate(vec3(0.0f, 2.0f * delta_time, 0.0f));
			orbit(m, sun, e.first, delta_time);
		}
		// All planets simply orbit the sun/black hole
		else
		{
			orbit(m, sun, e.first, delta_time);
		}
	}
	// Update the camera
	// Use keyboard to change camera location
	// 1 - (50, 10, 50)
	if (glfwGetKey(renderer::get_window(), '1')) {
		tcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	}
	// 2 - (10, 60, 10)
	if (glfwGetKey(renderer::get_window(), '2')) {
		tcam.set_position(vec3(10.0f, 60.0f, 10.0f));
	}
	tcam.update(delta_time);

	// *********************************
	// Use this to focus on object of choice
	// cam.set_target(meshes[].get_transform().position);
	// *********************************

	// Check to see if the sun has been clicked to destroy the solar system

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

		auto P = tcam.get_projection();
		auto V = tcam.get_view();
		auto inverse_matrix = inverse(P * V);

		vec4 ray_start_world = inverse_matrix * ray_start_screen;
		ray_start_world = ray_start_world / ray_start_world.w;
		vec4 ray_end_world = inverse_matrix * ray_end_screen;
		ray_end_world = ray_end_world / ray_end_world.w;

		direction = normalize(ray_end_world - ray_start_world);
		origin = ray_start_world;
		// Check all the mehes for intersection
		for (auto &m : meshes)
		{
			float distance = 0.0f;
			if (test_ray_oobb(origin, direction, m.second.get_minimal(), m.second.get_maximal(),
				m.second.get_transform().get_transform_matrix(), distance))
			{
				if (m.first == "sun")
				{
					destroy_solar_system = true;
				}
				cout << m.first << " " << distance << endl;
			}
		}
	}
	return true;
}

bool render() {
	// Render planets and sun
	auto V = tcam.get_view();
	auto P = tcam.get_projection();
	for (auto &e : meshes) {
		// Skip meshes: not to be rendered with eff,
		//				with scale of 0 (sucked into black hole),
		//				not to be rendered unless sun has been clicked
		if (e.first == "clouds" || 
			e.second.get_transform().scale == vec3(0.0f) ||
			(e.first == "black_hole" && destroy_solar_system == false))
		{
			continue;
		}
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		V = tcam.get_view();
		P = tcam.get_projection();
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Set N matrix uniform - remember - 3x3 matrix
		glUniformMatrix3fv(eff.get_uniform_location("N"),
			1,
			GL_FALSE,
			value_ptr(m.get_transform().get_normal_matrix()));
		// Bind material
		renderer::bind(m.get_material(), "mat");
		// Bind light
		renderer::bind(light, "point");
		// Bind and set textures
		renderer::bind(textures[e.first + "Tex"], 0);
		glUniform1i(eff.get_uniform_location("tex"), 0);
		// Set eye position- Get this from active camera
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(tcam.get_position()));
		// Render mesh
		renderer::render(m);
	}

	// Render clouds
	renderer::bind(ceff);
	// Create MVP matrix
	auto M = meshes["clouds"].get_transform().get_transform_matrix();
	V = tcam.get_view();
	P = tcam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Bind and set textures
	renderer::bind(textures["cloudsTex"], 0);
	glUniform1i(ceff.get_uniform_location("tex"), 0);
	// Render mesh
	renderer::render(meshes["clouds"]);

	// Render background
	glDisable(GL_CULL_FACE);
	renderer::bind(sbeff);
	MVP = P * V * scale(mat4(), vec3(200.0f));
	glUniformMatrix4fv(sbeff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	renderer::bind(cube_map, 0);
	glUniform1i(sbeff.get_uniform_location("cubemap"), 0);
	renderer::render(stars);
	glEnable(GL_CULL_FACE);
	
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