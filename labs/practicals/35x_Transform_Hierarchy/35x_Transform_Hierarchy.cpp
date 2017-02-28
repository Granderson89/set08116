#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

std::array<mesh, 7> meshes;
array<mesh, 2> motions;
array<texture, 2> motions_textures;
std::array<texture, 7> textures;
effect eff;
mesh plane_mesh;
texture plane_tex;
free_camera cam;
double cursor_x = 0.0;
double cursor_y = 0.0;

bool initialise() {
	// *********************************
	// Set input mode - hide the cursor
	GLFWwindow* window = renderer::get_window();
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(window, &cursor_x, &cursor_y);
	// *********************************
	return true;
}

bool load_content() {
  // Create plane mesh
  plane_mesh = mesh(geometry_builder::create_plane());

  // *********************************
  // Create Three Identical Box Meshes
  // Saucer section
  meshes[0] = mesh(geometry_builder::create_cylinder(20,50, vec3(10.0f, 1.0f, 10.0f)));
  // Connection
  meshes[1] = mesh(geometry_builder::create_box(vec3(1.0f, 4.0f, 1.0f)));
  // Shaft?
  meshes[2] = mesh(geometry_builder::create_cylinder(20,20, vec3(1.5f, 8.0f, 1.5f)));
  // Nacelles
  meshes[3] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.0f, 10.0, 1.0f)));
  meshes[4] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.0f, 10.0, 1.0f)));
  meshes[5] = mesh(geometry_builder::create_box(vec3(0.1f, 5.0f, 1.0f)));
  meshes[6] = mesh(geometry_builder::create_box(vec3(0.1f, 5.0f, 1.0f)));

  // Motions
  motions[0] = mesh(geometry_builder::create_sphere(20, 20, vec3(0.5f)));
  motions[1] = mesh(geometry_builder::create_sphere(20, 20, vec3(0.5f)));

  // Saucer section
  meshes[0].get_transform().position = vec3(0.0f, 10.0f, 0.0f);
  // Connection
  meshes[1].get_transform().position = vec3(0.0f, -2.5f, -4.5f);
  // Shaft?
  meshes[2].get_transform().position = vec3(0.0f, -2.0f, -3.25f);
  meshes[2].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  // Nacelles
  meshes[3].get_transform().position = vec3(3.0f, -4.0f, -5.0f);
  meshes[4].get_transform().position = vec3(-6.0f, 0.0f, 0.0f);
  // Nacelle connectors
  meshes[5].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));
  meshes[5].get_transform().position = vec3(1.5f, 1.5f, 2.5f);
  meshes[5].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>() / 3.0f ));
  meshes[6].get_transform().rotate(vec3(0.0f, 0.0f, -pi<float>() / 3.0f));
  meshes[6].get_transform().position = vec3(2.5f, -1.5f, 0.0f);

  motions[0].get_transform().position = meshes[0].get_transform().position - vec3(3.0f, -0.5f, 6.75f);
  motions[1].get_transform().position = meshes[0].get_transform().position - vec3(-3.0f, -0.5f, 6.75f);

  // Load texture
  plane_tex = texture("textures/snow.jpg");
  textures[0] = texture("textures/check_2.png");
  textures[1] = texture("textures/check_4.png");
  textures[2] = texture("textures/check_5.png");
  textures[3] = texture("textures/check_2.png");
  textures[4] = texture("textures/check_4.png");
  textures[5] = texture("textures/check_5.png");
  textures[6] = texture("textures/check_2.png");

  motions_textures[0] = texture("textures/check_4.png");
  motions_textures[1] = texture("textures/check_5.png");

  // Load in shaders
  eff.add_shader("27_Texturing_Shader/simple_texture.vert", GL_VERTEX_SHADER);
  eff.add_shader("27_Texturing_Shader/simple_texture.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_position(vec3(8.0f, 9.0f, 8.0f));
  cam.set_target(meshes[0].get_transform().position);
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
  return true;
}

bool update(float delta_time) {
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;
	// *********************************
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);

	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;

	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	cam.rotate(delta_x, delta_y);

	// Use keyboard to move the camera - WSAD
	vec3 dir;
	if (glfwGetKey(renderer::get_window(), 'W')) {
		dir += vec3(0.0f, 0.0f, 1.0f);
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		dir += vec3(0.0f, 0.0f, -1.0f);
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		dir += vec3(-1.0f, 0.0f, 0.0f);
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		dir += vec3(1.0f, 0.0f, 0.0f);
	}

	// Use keyboard to move the camera - WSAD
	vec3 engage;
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
		engage += vec3(0.0f, 0.0f, 1.0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
		engage += vec3(0.0f, 0.0f, -1.0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) {
		engage += vec3(-1.0f, 0.0f, 0.0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) {
		engage += vec3(1.0f, 0.0f, 0.0f);
	}

	meshes[0].get_transform().translate(engage);
	motions[0].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[1].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[0].get_transform().translate(engage);
	motions[1].get_transform().translate(engage);
	// Move camera
	cam.move(dir);

	// Update the camera
	cam.update(delta_time);

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

  return true;
}

bool render() {
  // Super effecient render loop, notice the things we only have to do once, rather than in a loop
  // Bind effect
  renderer::bind(eff);
  // Get PV
  const auto PV = cam.get_projection() * cam.get_view();
  // Set the texture value for the shader here
  glUniform1i(eff.get_uniform_location("tex"), 0);
  // Find the lcoation for the MVP uniform
  const auto loc = eff.get_uniform_location("MVP");

  // Render meshes
  for (size_t i = 0; i < meshes.size(); i++) {
    // *********************************
    // SET M to be the usual mesh  transform matrix
	  auto M = meshes[i].get_transform().get_transform_matrix();
    // *********************************

    // Apply the heirarchy chain
    for (size_t j = i; j > 0; j--) {
      M = meshes[j - 1].get_transform().get_transform_matrix() * M;
    }

    // Set MVP matrix uniform
    glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
    // Bind texture to renderer
    renderer::bind(textures[i], 0);
    // Render mesh
    renderer::render(meshes[i]);
  }
  for (int i = 0; i < motions.size(); i++)
  {
	  auto M = motions[i].get_transform().get_transform_matrix();
	  glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
	  renderer::bind(motions_textures[i], 0);
	  renderer::render(motions[i]);
  }
  // Render floor
  glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * plane_mesh.get_transform().get_transform_matrix()));
  // Bind floor texture
  renderer::bind(plane_tex, 0);
  // Render floor
  renderer::render(plane_mesh);
  return true;
}

void main() {
  // Create application
  app application("35_Geometry_Builder");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}