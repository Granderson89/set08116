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
vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord);


// Point lights for the scene
uniform point_light points[1];
// Material for the object
uniform material mat;
// Eye position
uniform vec3 eye_pos;
// Sampler used to get texture colour
uniform sampler2D tex;
// Normal map to sample from
uniform sampler2D normal_map;

// Incoming texture coordinate
layout(location = 0) in vec3 vertex_position;
// Incoming texture coordinate
layout(location = 1) in vec2 tex_coord_out;
// Incoming normal
layout(location = 2) in vec3 transformed_normal;
// Outgoing tangent
layout(location = 3) in vec3 tangent_out;
// Outgoing binormal
layout(location = 4) in vec3 binormal_out;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
	// Calculate view direction
	vec3 view_dir = normalize(eye_pos - vertex_position);
	// Sample texture
	vec4 tex_colour = texture(tex, tex_coord_out);
	// Calculate normal from normal map
	vec3 new_normal = calc_normal(transformed_normal, tangent_out, binormal_out, normal_map, tex_coord_out);
	// Sum point lights
	for (int i = 0; i < 1; ++i)
	{
		colour += calculate_point(points[i], mat, vertex_position, new_normal, view_dir, tex_colour);
	}
	// Set alpha to 1.0f
	colour.a = 1.0f;
	// Make cloudless areas transparent so Earth can be seen through gaps
	float transparency_factor = 0.3f;
	if (colour.r < transparency_factor && 
		colour.g < transparency_factor && 
		colour.b < transparency_factor)
	{
		colour.a = 0.0f;
	}
}