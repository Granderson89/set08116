#version 440

// Point light information
#ifndef POINT_LIGHT
#define POINT_LIGHT
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};
#endif

// Spot light data
#ifndef SPOT_LIGHT
#define SPOT_LIGHT
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};
#endif

// A material structure
#ifndef MATERIAL
#define MATERIAL
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};
#endif

// Forward declarations of used functions
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                     in vec4 tex_colour);
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir,
                    in vec4 tex_colour);
float calculate_shadow(in sampler2D shadow_map, in vec4 light_space_pos);

// Point lights for the scene
uniform point_light points[1];
// Spot lights for the scene
uniform spot_light spots[1];
// Material for the object
uniform material mat;
// Eye position
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex[2];
// Shadow map to sample from
uniform sampler2D shadow_map;

// Incoming position
layout(location = 0) in vec3 vertex_position;
// Incoming texture coordinate
layout(location = 1) in vec2 tex_coord_out;
// Incoming normal
layout(location = 2) in vec3 transformed_normal;
// Incoming light space position
layout(location = 5) in vec4 light_space_pos;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
	// Calculate shade factor
	float shade = calculate_shadow(shadow_map, light_space_pos);
	// Calculate view direction
	vec3 view_dir = normalize(eye_pos - vertex_position);
	// Sample texture 1
	vec4 tex_colour1 = texture(tex[0], tex_coord_out);
	// Sample texture 2
	vec4 tex_colour2 = texture(tex[1], tex_coord_out);

	// Mix the samples using weather factor
	vec4 tex_colour = mix(tex_colour1, tex_colour2, 0.5);

	// Sum point lights
	for (int i = 0; i < 1; ++i)
	{
		colour += calculate_point(points[i], mat, vertex_position, transformed_normal, view_dir, tex_colour);
	}
	// Sum spot lights (taking shadow into account)
	for (int i = 0; i < 1; ++i)
	{
		colour += calculate_spot(spots[i], mat, vertex_position, transformed_normal, view_dir, tex_colour) * shade;
	}
	// Set alpha to 1.0f
	colour.a = 1.0f;
}