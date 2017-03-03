#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh stars;
map<string, mesh> solar_objects;
cubemap cube_map;
array<mesh, 7> enterprise;
array<mesh, 2> motions;
mesh shadow_plane;
mesh rama;

effect planet_eff;
effect sbeff;
effect ceff;
effect suneff;
effect ship_eff;
effect inside_eff;
effect outside_eff;
effect shadow_eff;

map<string, texture> textures;
map<string, texture> normal_maps;
texture blend_map;
texture enterprise_texture;
array<texture, 2> motions_textures;

target_camera tcam;
free_camera fcam;
chase_camera ccam;
bool chase_camera_active = false;
bool free_camera_active = false;

directional_light directional;
vector<point_light> points(1);
vector<spot_light> spots(1);
vector<point_light> points_rama(1);
vector<spot_light> spots_rama(2);
shadow_map shadow;

map<string, float> orbit_factors;
double cursor_x = 0.0;
double cursor_y = 0.0;
float rotAngle = 0.0f;
bool destroy_solar_system = false;
bool demo_shadow = false;

// Define planetary orbits
void orbit(mesh &m, mesh &sun, string name, float delta_time)
{
	// Get centre of orbit, current position of orbiting
	// body and it's radius
	vec3 rotCenter = sun.get_transform().position;
	float current_x, current_z, rotAngle, radius;
	current_x = m.get_transform().position.x;
	current_z = m.get_transform().position.z;
	radius = distance(m.get_transform().position, rotCenter);
	// If planet has fallen into black hole, leave it there
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
	rotAngle = radians(rotAngle * (180.0f / pi<float>()) + 0.5f + orbit_factors[name]);
	// Correct for full rotation
	if (rotAngle > radians(360.0f))
		rotAngle = 0.0f;
	// Calculate new position
	float new_x = rotCenter.x + (radius * cosf(rotAngle));
	float new_z = rotCenter.z + (radius * sinf(rotAngle));
	vec3 newPos = vec3(new_x, 0, new_z);
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
void black_hole(float delta_time)
{
	// If the sun hasn't disappeared yet, shrink it to 0
	if (solar_objects["sun"].get_transform().scale != vec3(0.0f))
	{
		auto &m = solar_objects["sun"];
		vec3 sun_size = m.get_transform().scale;
		m.get_transform().scale = max(sun_size - vec3(delta_time, delta_time, delta_time), 0.0f);
	}
	// Else, grow the black hole to scale 20
	else
	{
		auto &m = solar_objects["black_hole"];
		vec3 black_hole_size = m.get_transform().scale;
		m.get_transform().scale = min(black_hole_size + vec3(delta_time, delta_time, delta_time), 20.0f);
	}
}

bool load_content() {
	// SOLAR OBJECT MESHES
	solar_objects["sun"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	solar_objects["mercury"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	solar_objects["venus"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	solar_objects["earth"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	solar_objects["mars"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	solar_objects["clouds"] = mesh(geometry(geometry_builder::create_sphere(20, 20)));
	solar_objects["black_hole"] = mesh(geometry(geometry_builder::create_disk(20)));

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

	//RAMA
	rama = mesh(geometry_builder::create_cylinder(100, 100));

	// DEMO SHADOW PLANE
	shadow_plane = mesh(geometry_builder::create_plane());

	// TRANSFORM MESHES
	// Solar objects
	solar_objects["sun"].get_transform().scale = vec3(8.0f, 8.0f, 8.0f);
	solar_objects["sun"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));
	solar_objects["sun"].get_transform().rotate(vec3(-half_pi<float>(), 0.0f, 0.0f));

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

	// Rama
	rama.get_transform().scale = vec3(10.0f, 20.0f, 10.0f);
	rama.get_transform().position = vec3(70.0f, 0.0f, 70.0f);
	rama.get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>()));

	// Demo shadow plane
	shadow_plane.get_transform().position = vec3(40.0f, -15.0f, 0.0f);
	shadow_plane.get_transform().scale = vec3(0.5f, 1.0f, 1.5f);

	// SET MATERIALS
	// Solar objects

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

	shadow_plane.get_material().set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shadow_plane.get_material().set_shininess(100.0f);

	solar_objects["sun"].get_material().set_emissive(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	solar_objects["sun"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	solar_objects["sun"].get_material().set_shininess(25.0f);

	solar_objects["black_hole"].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));

	// Enterprise
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

	// Rama
	rama.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	rama.get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	rama.get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	rama.get_material().set_shininess(25.0f);

	// Demo shadow plane
	shadow_plane.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	shadow_plane.get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shadow_plane.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 0.0f));
	shadow_plane.get_material().set_shininess(25.0f);

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
	textures["planeTex"] = texture("textures/white.jpg");


	// Enterprise
	enterprise_texture = texture("textures/saucer.jpg");
	motions_textures[0] = texture("textures/white.jpg");
	motions_textures[1] = texture("textures/white.jpg");

	// Rama
	textures["ramaOutTex"] = texture("textures/brick.jpg");
	textures["ramaInTex"] = texture("textures/Earth_tex.tga");
	textures["ramaGrassTex"] = texture("textures/grass.jpg");
	blend_map = texture("textures/blend_map.jpg");

	// LOAD NORMAL MAPS
	normal_maps["sun"] = texture("textures/sun_normal_map.png");
	normal_maps["black_hole"] = texture("textures/black_hole_normal_map.png");
	normal_maps["mercury"] = texture("textures/mercury_normal_map.png");
	normal_maps["venus"] = texture("textures/venus_normal_map.png");
	normal_maps["earth"] = texture("textures/earth_normal_map.png");
	normal_maps["mars"] = texture("textures/mars_normal_map.png");
	normal_maps["clouds"] = texture("textures/clouds_normal_map.png");
	normal_maps["saucer"] = texture("textures/saucer_normal_map.png");
	normal_maps["ramaOut"] = texture("textures/brick_normalmap.jpg");
	normal_maps["plane"] = texture("textures/plane_normal_map.png");

	// SKYBOX
	stars = mesh(geometry_builder::create_box());
	stars.get_transform().scale = vec3(1000.0f);
	array<string, 6> filenames = { "textures/stars_ft.jpg", "textures/stars_bk.jpg", "textures/stars_up.jpg", "textures/stars_dn.jpg", "textures/stars_lt.jpg", "textures/stars_rt.jpg" };
	cube_map = cubemap(filenames);

	// LIGHTS
	// Set point light values, Position
	points[0].move(vec3(0.0f, 0.0f, 0.0f));
	// Light colour white
	points[0].set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	// Set range to 20
	points[0].set_range(1000.0f);

	// Set spot light values
	// Yellow light
	spots[0].set_position(vec3(90.0f, 40.0f, 60.0f));
	spots[0].set_light_colour(vec4(1.0f, 1.0f, 0.0f, 1.0f));
	spots[0].set_direction(normalize(vec3(-1.0f, -1.0f, 0.0f)));
	spots[0].set_range(100.0f);
	spots[0].set_power(0.1f);

	// Set Rama lights
	points_rama[0].set_position(rama.get_transform().position);
	points_rama[0].set_light_colour(vec4(1.0f));
	points_rama[0].set_range(100.0f);

	spots_rama[0].set_position(rama.get_transform().position + vec3(5.0f, 0.0f, 0.0f));
	spots_rama[0].set_light_colour(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	spots_rama[0].set_direction(normalize(vec3(-1.0f, 0.0f, 0.0f)));
	spots_rama[0].set_range(70.0f);
	spots_rama[0].set_power(0.1f);

	spots_rama[1].set_position(rama.get_transform().position - vec3(5.0f, 0.0f, 0.0f));
	spots_rama[1].set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	spots_rama[1].set_direction(normalize(vec3(1.0f, 0.0f, 0.0f)));
	spots_rama[1].set_range(70.0f);
	spots_rama[1].set_power(0.1f);

	// SHADERS
	// Load in shaders for planets
	planet_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> planet_eff_frag_shaders{ "shaders/simple_texture.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag", "shaders/part_spot.frag" };
	planet_eff.add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	// Build effect
	planet_eff.build();
	
	// Load in shaders for clouds
	ceff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> ceff_frag_shaders{ "shaders/cloud_texture.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag"};
	ceff.add_shader(ceff_frag_shaders, GL_FRAGMENT_SHADER);
	ceff.build();
	
	// Load in shaders for sun
	suneff.add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	suneff.add_shader(planet_eff_frag_shaders, GL_FRAGMENT_SHADER);
	suneff.build();

	// Load in shaders for enterprise
	ship_eff.add_shader("shaders/enterprise.vert", GL_VERTEX_SHADER);
	vector<string> ship_eff_frag_shaders{ "shaders/enterprise.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	ship_eff.add_shader(ship_eff_frag_shaders, GL_FRAGMENT_SHADER);
	// Build effect
	ship_eff.build();

	// Load in shaders for rama
	inside_eff.add_shader("shaders/sun_shader.vert", GL_VERTEX_SHADER);
	vector<string> inside_eff_frag_shaders{ "shaders/inside.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	inside_eff.add_shader(inside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	inside_eff.build();

	// Load in shaders for rama
	outside_eff.add_shader("shaders/planet_shader.vert", GL_VERTEX_SHADER);
	vector<string> outside_eff_frag_shaders{ "shaders/blend.frag", "shaders/part_spot.frag", "shaders/part_point.frag", "shaders/part_shadow.frag", "shaders/part_normal_map.frag" };
	outside_eff.add_shader(outside_eff_frag_shaders, GL_FRAGMENT_SHADER);
	outside_eff.build();

	// Load in shaders for skybox
	sbeff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	sbeff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	sbeff.build();

	// Load is shadow shades
	shadow_eff.add_shader("shaders/spot.vert", GL_VERTEX_SHADER);
	shadow_eff.add_shader("shaders/spot.frag", GL_FRAGMENT_SHADER);
	shadow_eff.build();
	
	// SHADOW MAP
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());

	// CAMERAS
	// Set target camera properties
	tcam.set_position(vec3(60.0f, 10.0f, 60.0f));
	tcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	tcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	// Set free camera properties
	fcam.set_position(vec3(50.0f, 10.0f, 50.0f));
	fcam.set_target(vec3(0.0f, 0.0f, 0.0f));
	fcam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

	// Set chase camera properties
	ccam.set_pos_offset(vec3(0.0f, 2.0f, 10.0f));
	ccam.set_springiness(0.5f);
	ccam.move(solar_objects["mercury"].get_transform().position, eulerAngles(solar_objects["mercury"].get_transform().orientation));
	ccam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;
}

void chase_camera_update(float delta_time)
{
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// The target object for chase camera
	static mesh &target_mesh = solar_objects["earth"];
	// The ratio of pixels to rotation
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_height());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate camera by delta
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
	// The ratio of pixels to rotation
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_height());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;
	// Rotate cameras by delta
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
	
}

bool update(float delta_time) {
	// USER CONTROLS
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

	// Shadow plane controls
	if (glfwGetKey(renderer::get_window(), 'P'))
		demo_shadow = true;
	if (glfwGetKey(renderer::get_window(), 'O'))
		demo_shadow = false;

	// Check if solar system is to be destroyed
	if (destroy_solar_system == true)
		black_hole(delta_time);

	// Enterprise controls
	vec3 engage;
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) 
		engage += vec3(0.0f, 0.0f, 1.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) 
		engage += vec3(0.0f, 0.0f, -1.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) 
		engage += vec3(-1.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) 
		engage += vec3(1.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_PAGE_UP))
		engage += vec3(0.0f, 1.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_PAGE_DOWN))
		engage += vec3(0.0f, -1.0f, 0.0f);
	// Move enterprise
	enterprise[0].get_transform().translate(engage);
	motions[0].get_transform().translate(engage);
	motions[1].get_transform().translate(engage);
	// Spin nacelle domes
	motions[0].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	motions[1].get_transform().rotate(vec3(0.0f, 0.0f, 10.0f * delta_time));
	// Spin rama
	rama.get_transform().rotate(vec3(0.0f, delta_time, 0.0f));

	// ORBITS
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
			orbit(m, sun, e.first, delta_time);
		}
		// All planets simply orbit the sun
		else
			orbit(m, sun, e.first, delta_time);
	}

	// CAMERA MODES
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

	// Check for selection
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
		// Get inverse of PV
		auto P = tcam.get_projection();
		auto V = tcam.get_view();
		auto inverse_matrix = inverse(P * V);
		// Calculate origin and direction
		vec4 ray_start_world = inverse_matrix * ray_start_screen;
		ray_start_world = ray_start_world / ray_start_world.w;
		vec4 ray_end_world = inverse_matrix * ray_end_screen;
		ray_end_world = ray_end_world / ray_end_world.w;
		direction = normalize(ray_end_world - ray_start_world);
		origin = ray_start_world;
		// Check all the solar_objects for intersection
		for (auto &m : solar_objects)
		{
			float distance = 0.0f;
			if (test_ray_oobb(origin, direction, m.second.get_minimal(), m.second.get_maximal(),
				m.second.get_transform().get_transform_matrix(), distance))
			{
				if (m.first == "sun")
					destroy_solar_system = true;
			}
		}
	}
	// SHADOW MAP UPDATE
	shadow.light_position = spots[0].get_position();
	shadow.light_dir = spots[0].get_direction();

	// FPS
	cout << "FPS: " << 1.0f / delta_time << endl;
	return true;
}

void render_clouds(mat4 P, mat4 V, mat4 LightProjectionMat)
{
	// Render clouds
	renderer::bind(ceff);
	// Create MVP matrix
	auto M = solar_objects["clouds"].get_transform().get_transform_matrix();
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
		value_ptr(solar_objects["clouds"].get_transform().get_normal_matrix()));
	// *********************************
	// Set lightMVP uniform, using:
	//Model matrix from m
	// viewmatrix from the shadow map
	// Multiply together with LightProjectionMat
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	// Set uniform
	glUniformMatrix4fv(ceff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// *********************************
	// Bind material
	renderer::bind(solar_objects["clouds"].get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind and set textures
	renderer::bind(textures["cloudsTex"], 0);
	glUniform1i(ceff.get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(normal_maps["clouds"], 1);
	// Set normal_map uniform
	glUniform1i(ceff.get_uniform_location("normal_map"), 1);
	// Set eye position- Get this from active camera
	glUniform3fv(ceff.get_uniform_location("eye_pos"), 1, value_ptr(fcam.get_position()));
	// Bind shadow map texture - use texture unit 2
	renderer::bind(shadow.buffer->get_depth(), 2);
	// Set the shadow_map uniform
	glUniform1i(ceff.get_uniform_location("shadow_map"), 2);
	// Render mesh
	renderer::render(solar_objects["clouds"]);
}

void create_shadow_map(mat4 &LightProjectionMat)
{
	// Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);
	LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);
	// Bind shader
	renderer::bind(shadow_eff);
	// Render solar_objects
	for (size_t i = 0; i < enterprise.size(); i++) {
		// *********************************
		// SET M to be the usual mesh  transform matrix
		auto M = enterprise[i].get_transform().get_transform_matrix();
		// *********************************

		// Apply the heirarchy chain
		for (size_t j = i; j > 0; j--) {
			M = enterprise[j - 1].get_transform().get_transform_matrix() * M;
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
		renderer::render(enterprise[i]);
	}
	// Render solar_objects
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
	for (auto &e : solar_objects) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
			1,                                      // Number of values - 1 mat4
			GL_FALSE,                               // Transpose the matrix?
			value_ptr(MVP));                        // Pointer to matrix data
													// Render mesh
		renderer::render(m);
	}
	auto m = rama;
	auto M = m.get_transform().get_transform_matrix();
	auto V = shadow.get_view();
	auto MVP = LightProjectionMat * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"), // Location of uniform
		1,                                      // Number of values - 1 mat4
		GL_FALSE,                               // Transpose the matrix?
		value_ptr(MVP));                        // Pointer to matrix data
												// Render mesh
	renderer::render(m);
	// Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);

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

void render_solar_objects(effect eff, mesh m, string name, mat4 P, mat4 V, mat4 LightProjectionMat, vec3 cam_pos)
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
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	// Set uniform
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind spot light
	renderer::bind(spots, "spots");
	// Bind and set textures
	renderer::bind(textures[name + "Tex"], 0);
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(normal_maps[name], 1);
	// Set normal_map uniform
	glUniform1i(eff.get_uniform_location("normal_map"), 1);
	// Set eye position- Get this from active camera
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind shadow map texture - use texture unit 2
	renderer::bind(shadow.buffer->get_depth(), 2);
	// Set the shadow_map uniform
	glUniform1i(eff.get_uniform_location("shadow_map"), 2);
	// Render mesh
	renderer::render(m);
}

void render_enterprise(mat4 &LightProjectionMat, mat4 P, mat4 V, vec3 cam_pos)
{
	// Super effecient render loop, notice the things we only have to do once, rather than in a loop
	// Bind effect
	renderer::bind(ship_eff);
	// Get PV
	const auto PV = P * V;
	// Set the texture value for the shader here
	glUniform1i(ship_eff.get_uniform_location("tex"), 0);
	// Find the lcoation for the MVP uniform
	const auto loc = ship_eff.get_uniform_location("MVP");

	// Render solar_objects
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
		renderer::bind(enterprise[i].get_material(), "mat");
		// Bind point light
		renderer::bind(points, "points");
		// Bind spot light
		renderer::bind(spots, "spots");
		// Bind texture to renderer
		renderer::bind(enterprise_texture, 0);
		// Bind normal_map
		renderer::bind(normal_maps["saucer"], 1);
		// Set normal_map uniform
		glUniform1i(ship_eff.get_uniform_location("normal_map"), 1);
		// Set eye position
		glUniform3fv(ship_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 2);
		// Set the shadow_map uniform
		glUniform1i(ship_eff.get_uniform_location("shadow_map"), 2);
		// Render mesh
		renderer::render(enterprise[i]);
	}
	for (int i = 0; i < motions.size(); i++)
	{
		auto M = motions[i].get_transform().get_transform_matrix();
		glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(PV * M));
		// Set M matrix uniform
		glUniformMatrix4fv(ship_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		// Set N matrix uniform
		glUniformMatrix3fv(ship_eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(motions[i].get_transform().get_normal_matrix()));
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
		renderer::bind(motions[i].get_material(), "mat");
		// Bind point light
		renderer::bind(points, "points");
		// Bind spot light
		renderer::bind(spots, "spost");
		renderer::bind(motions_textures[i], 0);
		// Set eye position
		glUniform3fv(ship_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 1);
		// Set the shadow_map uniform
		glUniform1i(ship_eff.get_uniform_location("shadow_map"), 1);
		renderer::render(motions[i]);
	}
}

void render_rama(mat4 &LightProjectionMat, mat4 P, mat4 V, vec3 cam_pos)
{
	// Bind effect
	renderer::bind(outside_eff);
	// Create MVP matrix
	auto M = rama.get_transform().get_transform_matrix();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(outside_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(outside_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(outside_eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(rama.get_transform().get_normal_matrix()));
	// Set lightMVP uniform
	auto lightMVP = LightProjectionMat * shadow.get_view() * M;
	// Set uniform
	glUniformMatrix4fv(outside_eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(rama.get_material(), "mat");
	// Bind light
	renderer::bind(points, "points");
	// Bind light
	renderer::bind(spots, "spots");
	// Bind textures
	renderer::bind(textures["ramaOutTex"], 0);
	renderer::bind(textures["ramaGrassTex"], 1);
	// Bind blend map
	renderer::bind(blend_map, 2);
	// Set the uniform values for textures
	static int tex_indices[] = { 0, 1 };
	// Set tex uniform
	glUniform1iv(outside_eff.get_uniform_location("tex"), 2, tex_indices);
	// Set blend map uniform
	glUniform1i(outside_eff.get_uniform_location("blend_map"), 2);
	// Bind normal_map
	renderer::bind(normal_maps["ramaOut"], 3);
	// Set normal_map uniform
	glUniform1i(outside_eff.get_uniform_location("normal_map"), 3);
	// Set eye position
	glUniform3fv(outside_eff.get_uniform_location("eye_pos"), 1, value_ptr(cam_pos));
	// Bind shadow map texture - use texture unit 2
	renderer::bind(shadow.buffer->get_depth(), 4);
	// Set the shadow_map uniform
	glUniform1i(outside_eff.get_uniform_location("shadow_map"), 4);
	// Render mesh
	renderer::render(rama);

	// Disable depth test,depth mask,face culling
	glDisable(GL_CULL_FACE);
	// Bind effect
	renderer::bind(inside_eff);
	// Set MVP matrix uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
	// Set N matrix uniform
	glUniformMatrix3fv(inside_eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(rama.get_transform().get_normal_matrix()));
	// Set uniform
	glUniformMatrix4fv(inside_eff.get_uniform_location("lightMVP"),
		1,
		GL_FALSE,
		value_ptr(lightMVP));
	// Bind material
	renderer::bind(solar_objects["earth"].get_material(), "mat");
	// Bind light
	renderer::bind(points_rama, "points");
	// Bind light
	renderer::bind(spots_rama, "spots");
	// Bind texture
	renderer::bind(textures["ramaInTex"], 0);
	// Set tex uniform
	glUniform1i(inside_eff.get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(normal_maps["earth"], 1);
	// Set normal_map uniform
	glUniform1i(inside_eff.get_uniform_location("normal_map"), 1);
	renderer::render(rama);
	glEnable(GL_CULL_FACE);
}

bool render() {
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
	// Render to shadow map
	mat4 LightProjectionMat;
	create_shadow_map(LightProjectionMat);
	// Render solar objects
	for (auto &e : solar_objects) {
		// Skip those: with scale of 0 (sucked into black hole),
		//		       not to be rendered unless sun has been clicked
		if (e.first == "clouds" ||
			e.second.get_transform().scale == vec3(0.0f) ||
			(e.first == "black_hole" && destroy_solar_system == false))
		{
			continue;
		}
		// Render sun
		else if (e.first == "sun")
			render_solar_objects(suneff, e.second, e.first, P, V, LightProjectionMat, cam_pos);
		// Render planets
		else
			render_solar_objects(planet_eff, e.second, e.first, P, V, LightProjectionMat, cam_pos);
	}
	// Render clouds (if the earth hasn't been sucked into the black hole)
	if (solar_objects["earth"].get_transform().scale != vec3(0.0f))
		render_clouds(P, V, LightProjectionMat);
	// Render the Enterprise
	render_enterprise(LightProjectionMat, P, V, cam_pos);
	// Render shadow plane if necessary
	if (demo_shadow == true)
		render_solar_objects(planet_eff, shadow_plane, "plane", P, V, LightProjectionMat, cam_pos);
	// Render rama
	render_rama(LightProjectionMat, P, V, cam_pos);
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