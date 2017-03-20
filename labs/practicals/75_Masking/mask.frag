#version 430 core

// Captured render
uniform sampler2D tex;
// Alpha map
uniform sampler2D alpha_map;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample textures
  vec4 tex_sample = texture(tex, tex_coord);
  vec4 alpha_sample = texture(alpha_map, tex_coord);
  // Final colour is produce of these two colours
  colour = tex_sample * alpha_sample;
  // Ensure alpha is 1
  colour.a = 1.0f;
  // *********************************
}