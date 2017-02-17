#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
geometry geom4;
map<string, mesh> meshes;
effect eff;
effect sbeff;
map<string, texture> textures;
texture tex;
target_camera cam;
cubemap cube_map;
float rotAngle = 0.0f;

bool load_content() {
	// Create scene
	meshes["earth"] = mesh(geometry("models/Earth.obj"));
    //meshes["cylinder"] = mesh(geometry_builder::create_cylinder(20, 20));
	meshes["sun"] = mesh(geometry("models/Earth.obj"));

	// Transform objects
	meshes["earth"].get_transform().scale = vec3(1.0f, 1.0f, 1.0f);
	meshes["earth"].get_transform().translate(vec3(10.0f, 0.0f, -30.0f));
	//meshes["cylinder"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	//meshes["cylinder"].get_transform().translate(vec3(-25.0f, 0.0f, -25.0f));
	meshes["sun"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["sun"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));

	geom4 = geometry_builder::create_box();

	// Load texture
	textures["earthTex"] = texture("textures/Earth_tex.tga");
	textures["sunTex"] = texture("textures/sun_tex.jpg");

	string filenames = "textures/stars.jpg";
	cube_map = cubemap({filenames, filenames, filenames, filenames, filenames, filenames});

	// Load in shaders
	eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	sbeff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sbeff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	sbeff.build();

	// Set camera properties
	cam.set_position(vec3(50.0f, 10.0f, 50.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;
}


bool update(float delta_time) {
	// All othere meshes rotate around "hole"
	auto hole = meshes["sun"];
	for (auto &e : meshes)
	{
		if (e.first != "sun")
		{
			auto &m = e.second;
			vec3 rotCenter = hole.get_transform().position;
			float current_x, current_z, rotAngle, radius;
			current_x = m.get_transform().position.x;
			current_z = m.get_transform().position.z;
			radius = distance(m.get_transform().position, rotCenter);
			// If planet has fallen into black hole, leave it there
			if (radius < 0.1f)
			{
				continue;
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
			rotAngle++;
			rotAngle = radians(rotAngle);
			float factor = 1.0f / (radius * radius);
			cout << e.first << " " << radius << " " << rotAngle << endl;
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
			m.get_transform().translate(-factor * m.get_transform().position);
		}
	}
	// Update the camera
	cam.update(delta_time);
	return true;
}

bool render() {
	// Render meshes
	auto V = cam.get_view();
	auto P = cam.get_projection();
	for (auto &e : meshes) {
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		V = cam.get_view();
		P = cam.get_projection();
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

		// Bind and set texture
		renderer::bind(textures[e.first + "Tex"], 0);
		glUniform1i(eff.get_uniform_location("tex"), 0);

		// Render mesh
		renderer::render(m);
	}
	glDisable(GL_CULL_FACE);

	renderer::bind(sbeff);
	auto MVP = P * V * scale(mat4(), vec3(100.0f));
	glUniformMatrix4fv(sbeff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	renderer::bind(cube_map, 0);
	glUniform1i(sbeff.get_uniform_location("cubemap"), 0);
	renderer::render(geom4);

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