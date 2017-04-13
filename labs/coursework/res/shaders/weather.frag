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
uniform sampler2D tex[14];
// Shadow map to sample from
uniform sampler2D shadow_map;
// Weather factor
uniform float weather_factor;

// Incoming texture coordinate
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
	// Sample texture 3
	vec4 tex_colour3 = texture(tex[2], tex_coord_out);
	// Sample texture 4
	vec4 tex_colour4 = texture(tex[3], tex_coord_out);
	// Sample texture 5
	vec4 tex_colour5 = texture(tex[4], tex_coord_out);
	// Sample texture 6
	vec4 tex_colour6 = texture(tex[5], tex_coord_out);
	// Sample texture 7
	vec4 tex_colour7 = texture(tex[6], tex_coord_out);
	// Sample texture 8
	vec4 tex_colour8 = texture(tex[7], tex_coord_out);
	// Sample texture 9
	vec4 tex_colour9 = texture(tex[8], tex_coord_out);
	// Sample texture 10
	vec4 tex_colour10 = texture(tex[9], tex_coord_out);
	// Sample texture 11
	vec4 tex_colour11 = texture(tex[10], tex_coord_out);
	// Sample texture 12
	vec4 tex_colour12 = texture(tex[11], tex_coord_out);
	// Sample texture 13
	vec4 tex_colour13 = texture(tex[12], tex_coord_out);
	// Sample texture 14
	vec4 tex_colour14 = texture(tex[13], tex_coord_out);

	// Mix the samples using weather factor
	vec4 tex_colour;
	if (weather_factor < 0.07)
	{
		tex_colour = mix(tex_colour1, tex_colour2, 0.5);
	}
	else if (weather_factor < 0.14)
	{
		tex_colour = mix(tex_colour2, tex_colour3, 0.5);
	}
	else if (weather_factor < 0.21)
	{
		tex_colour = mix(tex_colour3, tex_colour4, 0.5);
	}
	else if (weather_factor < 0.28)
	{
		tex_colour = mix(tex_colour4, tex_colour5, 0.5);
	}
	else if (weather_factor < 0.35)
	{
		tex_colour = mix(tex_colour5, tex_colour6, 0.5);
	}
	else if (weather_factor < 0.42)
	{
		tex_colour = mix(tex_colour6, tex_colour7, 0.5);
	}
	else if (weather_factor < 0.49)
	{
		tex_colour = mix(tex_colour7, tex_colour8, 0.5);
	}
	else if (weather_factor < 0.56)
	{
		tex_colour = mix(tex_colour8, tex_colour9, 0.5);
	}
	else if (weather_factor < 0.63)
	{
		tex_colour = mix(tex_colour9, tex_colour10, 0.5);
	}
	else if (weather_factor < 0.70)
	{
		tex_colour = mix(tex_colour10, tex_colour11, 0.5);
	}
	else if (weather_factor < 0.77)
	{
		tex_colour = mix(tex_colour11, tex_colour12, 0.5);
	}
	else if (weather_factor < 0.84)
	{
		tex_colour = mix(tex_colour12, tex_colour13, 0.5);
	}
	else if (weather_factor < 0.91)
	{
		tex_colour = mix(tex_colour13, tex_colour14, 0.5);
	}
	else
	{
		tex_colour = mix(tex_colour14, tex_colour1, 0.5);
	}

	// Sum point lights
	for (int i = 0; i < 1; ++i)
	{
		colour += calculate_point(points[i], mat, vertex_position, transformed_normal, view_dir, tex_colour);
	}
	// Sum spot lights
	for (int i = 0; i < 1; ++i)
	{
		colour += calculate_spot(spots[i], mat, vertex_position, transformed_normal, view_dir, tex_colour) * shade;
	}
	// Set alpha to 1.0f
	colour.a = 1.0f;
}