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
effect shadow_eff;
mesh plane_mesh;
mesh test_plane;
texture plane_tex;
free_camera cam;
vector<point_light> points(1);
vector<spot_light> spots(1);
shadow_map shadow;

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
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
  // Create plane mesh
  plane_mesh = mesh(geometry_builder::create_plane());
  plane_mesh.get_transform().position = vec3(35.0f, 5.0f, 35.0f);
  plane_mesh.get_transform().rotate(vec3(0.0f, 0.0f, -half_pi<float>()));
  plane_mesh.get_transform().scale = vec3(0.25f);
  test_plane = mesh(geometry_builder::create_plane());
  test_plane.get_transform().position = vec3(0.0f, -10.0f, 0.0f);

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
  meshes[0].get_transform().position = vec3(50.0f, 10.0f, 50.0f);
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

  for (int i = 0; i < meshes.size(); i++)
  {
	  meshes[i].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	  meshes[i].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	  meshes[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	  meshes[i].get_material().set_shininess(25.0f);
  }
  for (int i = 0; i < motions.size(); i++)
  {
	  motions[i].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	  motions[i].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	  motions[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	  motions[i].get_material().set_shininess(25.0f);
  }
  plane_mesh.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  plane_mesh.get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
  plane_mesh.get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  plane_mesh.get_material().set_shininess(25.0f);
  // Red teapot
  // Set point light values, Position
  points[0].move(vec3(0.0f, 1.0f, 0.0f));
  // Light colour white
  points[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  // Set range to 20
  points[0].set_range(1000.0f);
  // *******************
  // Set spot properties
  // *******************
  // Pos (20, 30, 0), White
  // Direction (-1, -1, 0) normalized
  // 50 range, 10 power
  spots[0].set_position(vec3(90.0f, 40.0f, 60.0f));
  spots[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  spots[0].set_direction(normalize(vec3(-1.0f, -1.0f, 0.0f)));
  spots[0].set_range(1000.0f);
  spots[0].set_power(10.0f);

  // Load in shaders
  eff.add_shader("54_Shadowing/shadow.vert", GL_VERTEX_SHADER);
  vector<string> frag_shaders{ "54_Shadowing/shadow.frag", "shaders/part_spot.frag", "shaders/part_shadow.frag", "shaders/part_point.frag" };
  eff.add_shader(frag_shaders, GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  shadow_eff.add_shader("50_Spot_Light/spot.vert", GL_VERTEX_SHADER);
  shadow_eff.add_shader("50_Spot_Light/spot.frag", GL_FRAGMENT_SHADER);
  shadow_eff.build();

  // Set camera properties
  cam.set_position(vec3(50.0f, 10.0f, 50.0f));
  cam.set_target((vec3(0.0f, 0.0f, 0.0f)));
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
	// *********************************
	// Update the shadow map light_position from the spot light
	shadow.light_position = spots[0].get_position();
	// do the same for light_dir property
	shadow.light_dir = spots[0].get_direction();
	// *********************************
	// Press s to save
	if (glfwGetKey(renderer::get_window(), 'S') == GLFW_PRESS)
		shadow.buffer->save("test.png");
  return true;
}

void create_shadow_map(mat4 &LightProjectionMat)
{
	// *********************************
	// Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);
	// *********************************

	// We could just use the Camera's projection, 
	// but that has a narrower FoV than the cone of the spot light, so we would get clipping.
	// so we have yo create a new Proj Mat with a field of view of 90.
	LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);

	// Bind shader
	renderer::bind(shadow_eff);
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
		// *********************************
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
													// Render mesh
		renderer::render(meshes[i]);
	}
	// Render meshes
	for (int i = 0; i < motions.size(); i++) {
		auto m = motions[i];
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// *********************************
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
													// Render mesh
		renderer::render(m);
	}
	// *********************************
	// Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);
	// *********************************

}

void render_enterprise(mat4 &LightProjectionMat)
{
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
		// Set M matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(meshes[i].get_transform().get_normal_matrix()));
		// *********************************
		// Set lightMVP uniform, using:
		//Model matrix from m
		// viewmatrix from the shadow map
		// Multiply together with LightProjectionMat
		auto lightMVP = LightProjectionMat * shadow.get_view() * M;
		// Set uniform
		glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
			1,
			GL_FALSE,
			value_ptr(lightMVP));
		// Bind material
		renderer::bind(meshes[i].get_material(), "mat");
		// Bind point light
		renderer::bind(points[0], "point");
		// Bind spot light
		renderer::bind(spots[0], "spot");
		// Bind texture to renderer
		renderer::bind(textures[i], 0);
		// Set eye position
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 1);
		// Set the shadow_map uniform
		glUniform1i(eff.get_uniform_location("shadow_map"), 1);
		// Render mesh
		renderer::render(meshes[i]);
	}
	for (int i = 0; i < motions.size(); i++)
	{
		auto M = motions[i].get_transform().get_transform_matrix();
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
		// Set M matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(motions[i].get_transform().get_normal_matrix()));
		// *********************************
		// Set lightMVP uniform, using:
		//Model matrix from m

		// viewmatrix from the shadow map

		// Multiply together with LightProjectionMat
		auto lightMVP = LightProjectionMat * shadow.get_view() * M;
		// Set uniform
		glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
			1,
			GL_FALSE,
			value_ptr(lightMVP));
		// Bind material
		renderer::bind(motions[i].get_material(), "mat");
		// Bind point light
		renderer::bind(points[0], "point");
		// Bind spot light
		renderer::bind(spots[0], "spot");
		renderer::bind(motions_textures[i], 0);
		// Set eye position
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 1);
		// Set the shadow_map uniform
		glUniform1i(eff.get_uniform_location("shadow_map"), 1);
		renderer::render(motions[i]);
	}
}

void render_floor(mat4 &LightProjectionMat)
{
	// Render floor
	const auto PV = cam.get_projection() * cam.get_view();

	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(PV * plane_mesh.get_transform().get_transform_matrix()));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(plane_mesh.get_transform().get_transform_matrix()));
	// Set N matrix uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(plane_mesh.get_transform().get_normal_matrix()));
	// *********************************
	// Set lightMVP uniform, using:
	//Model matrix from m

	// viewmatrix from the shadow map

	// Multiply together with LightProjectionMat
	auto lightMVP = LightProjectionMat * shadow.get_view() * plane_mesh.get_transform().get_transform_matrix();
	// Set uniform
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(plane_mesh.get_material(), "mat");
	// Bind point light
	renderer::bind(points[0], "point");
	// Bind spot light
	renderer::bind(spots[0], "spot");
	// Bind floor texture
	renderer::bind(plane_tex, 0);
	// Set tex uniform
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Set eye position
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	// Bind shadow map texture - use texture unit 1
	renderer::bind(shadow.buffer->get_depth(), 1);
	// Set the shadow_map uniform
	glUniform1i(eff.get_uniform_location("shadow_map"), 1);
	// Render floor
	renderer::render(plane_mesh);
}

bool render() {
	mat4 LightProjectionMat;
	create_shadow_map(LightProjectionMat);
	render_enterprise(LightProjectionMat);
	render_floor(LightProjectionMat);
	// Render floor
	const auto PV = cam.get_projection() * cam.get_view();

	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(PV * test_plane.get_transform().get_transform_matrix()));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(test_plane.get_transform().get_transform_matrix()));
	// Set N matrix uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(test_plane.get_transform().get_normal_matrix()));
	// *********************************
	// Set lightMVP uniform, using:
	//Model matrix from m

	// viewmatrix from the shadow map

	// Multiply together with LightProjectionMat
	auto lightMVP = LightProjectionMat * shadow.get_view() * test_plane.get_transform().get_transform_matrix();
	// Set uniform
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(test_plane.get_material(), "mat");
	// Bind point light
	renderer::bind(points[0], "point");
	// Bind spot light
	renderer::bind(spots[0], "spot");
	// Bind floor texture
	renderer::bind(plane_tex, 0);
	// Set tex uniform
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Set eye position
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
	// Bind shadow map texture - use texture unit 1
	renderer::bind(shadow.buffer->get_depth(), 1);
	// Set the shadow_map uniform
	glUniform1i(eff.get_uniform_location("shadow_map"), 1);
	// Render floor
	renderer::render(test_plane);
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