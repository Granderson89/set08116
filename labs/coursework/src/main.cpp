#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh stars;
map<string, geometry> geoms;
map<string, mesh> meshes;
map<string, float> orbit_factors;
cubemap cube_map;

effect eff;
effect sbeff;
effect ceff;
effect suneff;
effect shadoweff;

map<string, texture> textures;
texture blend_map;

target_camera tcam;
free_camera fcam;
chase_camera ccam;
bool chase_camera_active = false;
bool free_camera_active = false;

vector<point_light> points(1);
vector<spot_light> spots(1);
shadow_map shadow;

double cursor_x = 0.0;
double cursor_y = 0.0;
string selected = "earth";

float rotAngle = 0.0f;
bool destroy_solar_system = false;

effect ship_eff;
array<mesh, 7> enterprise;
array<mesh, 2> motions;
array<texture, 2> motions_textures;
array<texture, 7> enterprise_textures;

bool initialise() {
	// *********************************
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	// *********************************
	return true;
}

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
	rotAngle = rotAngle + orbit_factors[name];
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
	// Meshes
	meshes["earth"] = mesh(geometry("models/Earth.obj"));
	meshes["clouds"] = mesh(geometry("models/Earth.obj"));
	meshes["sun"] = mesh(geometry("models/Earth.obj"));
	meshes["black_hole"] = mesh(geometry(geometry_builder::create_disk(20)));
	meshes["alien"] = mesh((geometry(geometry_builder::create_box())));
	meshes["mercury"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	meshes["venus"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	meshes["mars"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	// Create shadow map- use screen size
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
	// Skybox
	stars = mesh(geometry_builder::create_box());
	stars.get_transform().scale = vec3(1000.0f);

	// Transform objects
	meshes["earth"].get_transform().scale = vec3(1.0f);
	meshes["earth"].get_transform().translate(vec3(20.0f, 0.0f, -40.0f));
	meshes["earth"].get_transform().rotate(vec3(0.0f, radians(23.44), 0.0f));
	meshes["clouds"].get_transform().scale = vec3(1.005f, 1.005f, 1.005f);
	meshes["clouds"].get_transform().position = meshes["earth"].get_transform().position;
	meshes["sun"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["sun"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));
	meshes["black_hole"].get_transform().rotate(vec3(0.5f, 0.0f, 0.0f));
	meshes["alien"].get_transform().translate(vec3(10.0f, 10.0f, -30.0f));
	meshes["venus"].get_transform().scale = vec3(2.0f);
	meshes["venus"].get_transform().translate(0.72f * meshes["earth"].get_transform().position);
	meshes["venus"].get_transform().rotate(vec3(0.0f, radians(90.0), 0.0f));
	meshes["mercury"].get_transform().scale = 0.3f * meshes["venus"].get_transform().scale;
	meshes["mercury"].get_transform().translate(0.39f * meshes["earth"].get_transform().position);
	meshes["mercury"].get_transform().rotate(vec3(0.0f, radians(90.0), 0.0f));
	meshes["mars"].get_transform().scale = 0.53f * meshes["venus"].get_transform().scale;
	meshes["mars"].get_transform().translate(1.52f * meshes["earth"].get_transform().position);
	meshes["mars"].get_transform().rotate(vec3(0.0f, radians(90.0), 0.0f));

	// Set orbit factors
	orbit_factors["mercury"] = 1.5f;
	orbit_factors["venus"] = 1.1f;
	orbit_factors["earth"] = 0.0f;
	orbit_factors["clouds"] = 0.0f;
	orbit_factors["mars"] = -0.235f;

	// Set materials
	material mat;
	mat.set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	meshes["earth"].set_material(mat);
	meshes["clouds"].set_material(mat);
	meshes["alien"].set_material(mat);
	meshes["venus"].set_material(mat);
	meshes["mars"].set_material(mat);

	mat.set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	mat.set_shininess(100.0f);
	meshes["mercury"].set_material(mat);

	mat.set_emissive(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	mat.set_shininess(25.0f);
	meshes["sun"].set_material(mat);

	mat.set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	meshes["black_hole"].set_material(mat);

	// Load textures
	textures["earthTex"] = texture("textures/Earth_tex.tga");
	textures["sunTex"] = texture("textures/sun_tex.jpg");
	textures["cloudsTex"] = texture("textures/clouds.png");
	textures["black_holeTex"] = texture("textures/blackhole.jpg");
	textures["alienTex"] = texture("textures/white.jpg");
	textures["mercuryTex"] = texture("textures/mercury_tex.jpg");
	textures["venusTex"] = texture("textures/venus.jpg");
	textures["marsTex"] = texture("textures/mars.jpg");
	// Create skybox
	array<string, 6> filenames = { "textures/stars_ft.jpg", "textures/stars_bk.jpg", "textures/stars_up.jpg", "textures/stars_dn.jpg", "textures/stars_lt.jpg", "textures/stars_rt.jpg" };
	cube_map = cubemap(filenames);

	// Set point light values, Position
	points[0].move(vec3(0.0f, 1.0f, 0.0f));
	// Light colour white
	points[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// Set range to 20
	points[0].set_range(1000.0f);

	// Set spot light values
	// Selection light (red)
	// Default position is above Earth
	spots[0].set_position(vec3(60.0f, 20.0f, 60.0f));
	spots[0].set_light_colour(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	// Point towards Earth
	spots[0].set_direction(normalize(vec3(-1.0f, -1.0f, 0.0f)));
	spots[0].set_range(1000.0f);
	spots[0].set_power(1.0f);

	// Load in shaders for planets
	eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	vector<string> eff_frag_shaders{ "shaders/simple_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag"};
	eff.add_shader(eff_frag_shaders, GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	// Load in shaders for skybox
	sbeff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sbeff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	sbeff.build();

	// Load in shaders for clouds
	ceff.add_shader("shaders/cloud_texture.vert", GL_VERTEX_SHADER);
	vector<string> ceff_frag_shaders{ "shaders/cloud_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag" };
	ceff.add_shader(ceff_frag_shaders, GL_FRAGMENT_SHADER);
	ceff.build();

	// Load in shaders for sun
	suneff.add_shader("shaders/sun_texture.vert", GL_VERTEX_SHADER);
	vector<string> seff_frag_shaders{ "shaders/simple_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag"};
	suneff.add_shader(seff_frag_shaders, GL_FRAGMENT_SHADER);
	suneff.build();

	// Load is shadow shades
	shadoweff.add_shader("shaders/multi-light.vert", GL_VERTEX_SHADER);
	shadoweff.add_shader("shaders/multi-light.frag", GL_FRAGMENT_SHADER);
	shadoweff.build();

	// *********************************
	// Create Three Identical Box Meshes
	// Saucer section
	enterprise[0] = mesh(geometry_builder::create_cylinder(20, 50, vec3(10.0f, 1.0f, 10.0f)));
	// Connection
	enterprise[1] = mesh(geometry_builder::create_box(vec3(1.0f, 4.0f, 1.0f)));
	// Shaft?
	enterprise[2] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.5f, 8.0f, 1.5f)));
	// Nacelles
	enterprise[3] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.0f, 10.0, 1.0f)));
	enterprise[4] = mesh(geometry_builder::create_cylinder(20, 20, vec3(1.0f, 10.0, 1.0f)));
	enterprise[5] = mesh(geometry_builder::create_box(vec3(0.1f, 5.0f, 1.0f)));
	enterprise[6] = mesh(geometry_builder::create_box(vec3(0.1f, 5.0f, 1.0f)));

	// Motions
	motions[0] = mesh(geometry_builder::create_sphere(20, 20, vec3(0.5f)));
	motions[1] = mesh(geometry_builder::create_sphere(20, 20, vec3(0.5f)));

	// Saucer section
	enterprise[0].get_transform().position = vec3(50.0f, 10.0f, 50.0f);
	// Connection
	enterprise[1].get_transform().position = vec3(0.0f, -2.5f, -4.5f);
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

	motions[0].get_transform().position = enterprise[0].get_transform().position - vec3(3.0f, -0.5f, 6.75f);
	motions[1].get_transform().position = enterprise[0].get_transform().position - vec3(-3.0f, -0.5f, 6.75f);

	// Load texture
	enterprise_textures[0] = texture("textures/check_2.png");
	enterprise_textures[1] = texture("textures/check_4.png");
	enterprise_textures[2] = texture("textures/check_5.png");
	enterprise_textures[3] = texture("textures/check_2.png");
	enterprise_textures[4] = texture("textures/check_4.png");
	enterprise_textures[5] = texture("textures/check_5.png");
	enterprise_textures[6] = texture("textures/check_2.png");

	motions_textures[0] = texture("textures/check_4.png");
	motions_textures[1] = texture("textures/check_5.png");

	// Load in shaders for enterprise
	ship_eff.add_shader("shaders/enterprise.vert", GL_VERTEX_SHADER);
	vector<string> ship_eff_frag_shaders{ "shaders/enterprise.frag", "shaders/part_spot.frag", "shaders/part_shadow.frag" };
	ship_eff.add_shader(ship_eff_frag_shaders, GL_FRAGMENT_SHADER);
	// Build effect
	ship_eff.build();

	// Set target camera properties
	tcam.set_position(vec3(60.0f, 10.0f, 60.0f));
	tcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	tcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	/*
	//TESTING
	tcam.set_position(vec3(10.0f, 20.0f, 10.0f));
	tcam.set_target(meshes["alien"].get_transform().position);
	*/
	// Set free camera properties
	fcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	fcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	fcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	// Set chase camera properties
	ccam.set_pos_offset(vec3(0.0f, 2.0f, 10.0f));
	ccam.set_springiness(0.5f);
	ccam.move(meshes["mercury"].get_transform().position, eulerAngles(meshes["mercury"].get_transform().orientation));
	ccam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;
}

void chase_camera_update(float delta_time)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// The target object for chase camera
	static mesh &target_mesh = meshes["mercury"];
	// The ratio of pixels to rotation - remember the fov
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_height());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;

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
	// x - delta_y
	// y - delta_x
	// z - 0
	ccam.rotate(vec3(delta_y, delta_x, 0.0f));

	// Move camera - update target position and rotation
	ccam.move(target_mesh.get_transform().position, vec3(0.0f, 0.0f, 0.0f));

	// Update the camera
	ccam.update(delta_time);

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	// Set skybox position to camera position (camera in centre of skybox)
	stars.get_transform().position = ccam.get_position();
}

void free_camera_update(float delta_time)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Update the camera
	// Use keyboard to change camera location
	// 1 - (50, 10, 50)
	if (glfwGetKey(renderer::get_window(), '1')) {
		fcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	}
	// 2 - (10, 60, 10)
	if (glfwGetKey(renderer::get_window(), '2')) {
		fcam.set_position(vec3(10.0f, 60.0f, 10.0f));
	}
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
	GLFWwindow* window = renderer::get_window();
	glfwGetCursorPos(window, &current_x, &current_y);

	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;

	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	fcam.rotate(delta_x, delta_y);

	// Use keyboard to move the camera - WSAD
	if (glfwGetKey(renderer::get_window(), 'W')) {
		fcam.move(vec3(0.0f, 0.0f, 1.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'S')) {
		fcam.move(vec3(0.0f, 0.0f, -1.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'A')) {
		fcam.move(vec3(-1.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'D')) {
		fcam.move(vec3(1.0f, 0.0f, 0.0f));
	}

	// Update the camera
	fcam.update(delta_time);

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	// Set skybox position to camera position (camera in centre of skybox)
	stars.get_transform().position = fcam.get_position();
}

void target_camera_update(float delta_time)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	// Use keyboard to change camera location
	// 1 - (50, 10, 50)
	if (glfwGetKey(renderer::get_window(), '1')) {
		tcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	}
	// 2 - (-50, 10, 50)
	if (glfwGetKey(renderer::get_window(), '2')) {
		tcam.set_position(vec3(-50.0f, 10.0f, 50.0f));
	}
	// 3 - (-50, 10, -50)
	if (glfwGetKey(renderer::get_window(), '3')) {
		tcam.set_position(vec3(-50.0f, 10.0f, -50.0f));
	}
	// 4 - (50, 10, -50)
	if (glfwGetKey(renderer::get_window(), '4')) {
		tcam.set_position(vec3(50.0f, 10.0f, -50.0f));
	}
	// Update the camera
	tcam.update(delta_time);
	// Set skybox position to camera position (camera in centre of skybox)
	stars.get_transform().position = tcam.get_position();
	// Get selected item

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
			}
		}
	}
}

bool update(float delta_time) {
	// *** USER CONTROLS ***
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
	// Alien ship controls (arrow keys)
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
		meshes["alien"].get_transform().translate(vec3(0.0f, 1.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
		meshes["alien"].get_transform().translate(vec3(0.0f, -1.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) {
		meshes["alien"].get_transform().translate(vec3(-1.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) {
		meshes["alien"].get_transform().translate(vec3(1.0f, 0.0f, 0.0f));
	}

	// *** MOTION ***
	// All planets orbit "sun"
	auto sun = meshes["sun"];
	// Control orbiting objects motion
	for (auto &e : meshes)
	{
		auto &m = e.second;
		// Ignore the alien ship
		if (e.first == "alien" || e.first == "plane")
		{
			continue;
		}
		// The sun and the black hole spin around the origin
		if (e.first == "sun" || e.first == "black_hole")
		{
			m.get_transform().rotate(vec3(0.0f, -delta_time / 2.0f, 0.0f));
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

	enterprise[0].get_transform().translate(engage);
	motions[0].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[1].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[0].get_transform().translate(engage);
	motions[1].get_transform().translate(engage);
	// Check if solar system is to be destroyed
	if (destroy_solar_system == true)
	{
		black_hole(delta_time);
	}
	// *********************************
	// Update the shadow map light_position from the spot light
	shadow.light_position = spots[0].get_position();
	// do the same for light_dir property
	shadow.light_dir = spots[0].get_direction();
	// *********************************
	// Press s to save
	if (glfwGetKey(renderer::get_window(), 'S') == GLFW_PRESS)
		shadow.buffer->save("test.png");
	// *** CAMERA MODE ***
	// Update depending on active camera
	if (chase_camera_active)
	{
		chase_camera_update(delta_time);
	}
	else if (free_camera_active)
	{
		free_camera_update(delta_time);
	}
	else
	{
		target_camera_update(delta_time);
	}
	cout << "FPS: " << 1.0f / delta_time << endl;

	return true;
}

void render_clouds(mat4 P, mat4 V)
{
	// Render clouds
	renderer::bind(ceff);
	// Create MVP matrix
	auto M = meshes["clouds"].get_transform().get_transform_matrix();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(ceff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(ceff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(ceff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(meshes["clouds"].get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(meshes["clouds"].get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind and set textures
	renderer::bind(textures["cloudsTex"], 0);
	glUniform1i(ceff.get_uniform_location("tex"), 0);
	// Set eye position- Get this from active camera
	glUniform3fv(ceff.get_uniform_location("eye_pos"), 1, value_ptr(fcam.get_position()));
	// Render mesh
	renderer::render(meshes["clouds"]);
}

void render_sun(mat4 P, mat4 V)
{
	// Bind effect
	renderer::bind(suneff);
	// Create MVP matrix
	auto M = meshes["sun"].get_transform().get_transform_matrix();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(suneff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(suneff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(meshes["sun"].get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(meshes["sun"].get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind and set textures
	renderer::bind(textures["sunTex"], 0);
	glUniform1i(suneff.get_uniform_location("tex"), 0);
	// Set eye position- Get this from active camera
	glUniform3fv(suneff.get_uniform_location("eye_pos"), 1, value_ptr(fcam.get_position()));
	// Render mesh
	renderer::render(meshes["sun"]);
}

void render_planets(mesh m, string name, mat4 P, mat4 V)
{
	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	auto M = m.get_transform().get_transform_matrix();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"),
		1,
		GL_FALSE,
		value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"),
		1,
		GL_FALSE,
		value_ptr(M));
	// Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(eff.get_uniform_location("N"),
		1,
		GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));
	// *********************************
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	if (name != "sun")
	{
		renderer::bind(points, "points");
	}
	// Bind and set textures
	renderer::bind(textures[name + "Tex"], 0);
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Set eye position- Get this from active camera
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(fcam.get_position()));
	// Bind shadow map texture - use texture unit 1
	renderer::bind(shadow.buffer->get_depth(), 1);
	// Set the shadow_map uniform
	glUniform1i(eff.get_uniform_location("shadow_map"), 1);
	// Render mesh
	renderer::render(m);
}

void render_skybox(mat4 P, mat4 V)
{
	// Disable depth test,depth mask,face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(sbeff);
	// Calculate MVP for the skybox
	auto MVP = P * V * stars.get_transform().get_transform_matrix();
	glUniformMatrix4fv(sbeff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	renderer::bind(cube_map, 0);
	glUniform1i(sbeff.get_uniform_location("cubemap"), 0);
	renderer::render(stars);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}

bool render() {
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
	mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);

	// Bind shader
	renderer::bind(shadoweff);
	// Render meshes
	for (auto &e : meshes) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// *********************************
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadoweff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
													// Render mesh
		renderer::render(m);
	}
	// Render meshes
	for (size_t i = 0; i < enterprise.size(); i++) {
		// *********************************
		// SET M to be the usual mesh  transform matrix
		auto M = enterprise[i].get_transform().get_transform_matrix();
		// *********************************

		// Apply the heirarchy chain
		for (size_t j = i; j > 0; j--) {
			M = enterprise[j - 1].get_transform().get_transform_matrix() * M;
		}
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadoweff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// Render mesh
		renderer::render(enterprise[i]);
	}
	for (int i = 0; i < motions.size(); i++)
	{
		auto M = motions[i].get_transform().get_transform_matrix();
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		// *********************************
		auto MVP = LightProjectionMat * V * M;
		glUniformMatrix4fv(shadoweff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		renderer::render(motions[i]);
	}
	// *********************************
	// Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);
	// *********************************

	// *********************************
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
	// Render background
	render_skybox(P, V);
	// Render objects
	for (auto &e : meshes) {
		// Skip meshes: with scale of 0 (sucked into black hole),
		//				not to be rendered unless sun has been clicked
		if (e.first == "clouds" ||
			e.second.get_transform().scale == vec3(0.0f) ||
			e.first == "alien_beam" ||
			(e.first == "black_hole" && destroy_solar_system == false))
		{
			continue;
		}
		// Render sun
		else if (e.first == "sun")
		{
			render_sun(P, V);
		}
		// Render planets
		else
		{
			render_planets(e.second, e.first, P, V);
		}
	}
	// Render clouds
	render_clouds(P, V);

	// Render Enterprise
	// Bind effect
	renderer::bind(ship_eff);
	// Get PV
	const auto PV = P * V;
	// Set the texture value for the shader here
	glUniform1i(ship_eff.get_uniform_location("tex"), 0);
	// Find the lcoation for the MVP uniform
	const auto loc = ship_eff.get_uniform_location("MVP");
	// Render meshes
	for (size_t i = 0; i < enterprise.size(); i++) {
		// *********************************
		// SET M to be the usual mesh  transform matrix
		auto M = enterprise[i].get_transform().get_transform_matrix();
		// *********************************

		// Apply the heirarchy chain
		for (size_t j = i; j > 0; j--) {
			M = enterprise[j - 1].get_transform().get_transform_matrix() * M;
		}

		// Set MVP matrix uniform
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
		// Set M matrix uniform
		glUniformMatrix4fv(ship_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(ship_eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(enterprise[i].get_transform().get_normal_matrix()));
		// *********************************
		// Set lightMVP uniform, using:
		//Model matrix from m
		// viewmatrix from the shadow map
		// Multiply together with LightProjectionMat
		auto lightMVP = LightProjectionMat * shadow.get_view() * M;
		// Set uniform
		glUniformMatrix4fv(ship_eff.get_uniform_location("lightMVP"),
			1,
			GL_FALSE,
			value_ptr(lightMVP));
		// Bind material
		renderer::bind(meshes["earth"].get_material(), "mat");
		// Bind spot light
		renderer::bind(spots[0], "spot");
		// Bind texture to renderer
		renderer::bind(enterprise_textures[i], 0);
		// Set eye position
		glUniform3fv(ship_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 1);
		// Set the shadow_map uniform
		glUniform1i(ship_eff.get_uniform_location("shadow_map"), 1);
		// Render mesh
		renderer::render(enterprise[i]);
	}
	for (int i = 0; i < motions.size(); i++)
	{
		auto M = motions[i].get_transform().get_transform_matrix();
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
		renderer::bind(motions_textures[i], 0);
		renderer::render(motions[i]);
	}
	return true;
}



void main() {
	// Create application
	app application("Graphics Coursework");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_initialise(initialise);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}