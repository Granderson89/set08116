#version 440

// MVP transformation matrix
uniform mat4 MVP;
// MV transformation matrix
uniform mat4 MV;
// M transformation matrix
uniform mat4 M;
// N transformation matrix
uniform mat3 N;
// The light transformation matrix
uniform mat4 lightMVP;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming texture coordinate
layout(location = 10) in vec2 tex_coord;
// Incoming texture weight
layout(location = 11) in vec4 tex_weight;

// Outgoing vertex position
layout(location = 0) out vec3 vertex_position;
// Transformed normal
layout(location = 1) out vec3 transformed_normal;
// Outgoing tex_coord
layout(location = 2) out vec2 vertex_tex_coord;
// Outgoing tex_weight
layout(location = 3) out vec4 vertex_tex_weight;
// Outgoing light space position
layout(location = 5) out vec4 light_space_pos;
// Outgoing camera space position
layout(location = 6) out vec4 CS_position;

void main() {
  // Calculate screen position
  gl_Position = MVP * vec4(position, 1.0);
  // Calculate camera space position
  CS_position = MV * vec4(position, 1.0);
  // Calculate vertex world position
  vertex_position = (M * vec4(position, 1.0)).xyz;
  // Transform normal
  transformed_normal = N * normal;
  // Pass through tex_coord
  vertex_tex_coord = tex_coord;
  // Pass through tex_weight
  vertex_tex_weight = tex_weight;
  // Transform position into light space
  light_space_pos = lightMVP * vec4(position, 1.0f);
}