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

// Point light for the scene
uniform point_light points[1];
// Material for the object
uniform material mat;
// Eye position
uniform vec3 eye_pos;
// Sampler used to get texture colour
uniform sampler2D tex;

// Incoming texture coordinate
layout(location = 0) in vec3 vertex_position;
// Incoming normal
layout(location = 1) in vec3 transformed_normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord_out;

// Outgoing colour
layout(location = 0) out vec4 out_colour;

void main() {
  // *********************************
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - vertex_position);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord_out);
  for (int i = 0; i < 1; ++i)
  {
    out_colour += calculate_point(points[i], mat, vertex_position, transformed_normal, view_dir, tex_colour);
  }
  out_colour.a = 1.0f;
  float transparency_factor = 0.3f;
  if (out_colour.r < transparency_factor && 
      out_colour.g < transparency_factor && 
	  out_colour.b < transparency_factor)
  {
     out_colour.a = 0.0f;
  }
  // *********************************
}