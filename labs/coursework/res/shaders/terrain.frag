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
vec4 weighted_texture(in sampler2D tex[4], in vec2 tex_coord, in vec4 weights);
float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density,
                    in int fog_type);
float calculate_shadow(in sampler2D shadow_map, in vec4 light_space_pos);


// Point lights for the scene
uniform point_light points[1];
// Spot lights for the scene
uniform spot_light spots[1];
// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;
// Textures
uniform sampler2D tex[4];
// Fog colour
uniform vec4 fog_colour;
// Fog start position
uniform float fog_start;
// Fog end position
uniform float fog_end;
// Fog density
uniform float fog_density;
// Fog type
uniform int fog_type;
// Shadow map
uniform sampler2D shadow_map;

// Incoming vertex position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming tex_coord
layout(location = 2) in vec2 tex_coord;
// Incoming tex_weight
layout(location = 3) in vec4 tex_weight;
// Incoming light space position
layout(location = 5) in vec4 light_space_pos;
// Camera space position
layout(location = 6) in vec4 CS_position;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // Calculate shade factor
  float shade = calculate_shadow(shadow_map, light_space_pos);
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);
  // Get tex colour
  vec4 tex_colour = weighted_texture(tex, tex_coord, tex_weight);
  // Sum point lights
  for (int i = 0; i < 1; ++i)
  {
	colour += calculate_point(points[i], mat, position, normal, view_dir, tex_colour);
  }
  // Sum spot lights
  for (int i = 0; i < 1; ++i)
  {
	colour += calculate_spot(spots[i], mat, position, normal, view_dir, tex_colour);
  }
  // Calculate fog coord
  float fog_coord = abs(CS_position.z / CS_position.w);
  // Calculate fog factor
  float fog_factor = calculate_fog(fog_coord, fog_colour, fog_start, fog_end, fog_density, fog_type);
  // Colour is mix between colour and fog colour based on factor and taking shadow into account
  colour = mix(colour, fog_colour, fog_factor) * shade;
  // Set alpha to 1.0f
  colour.a = 1.0f;
}