#version 430 core

// Incoming texture containing frame information
uniform sampler2D tex;

// Our colour filter - calculates colour intensity
uniform vec3 intensity;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample texture colour
  vec4 tex_colour = texture(tex, tex_coord);
  // Calculate grey value
  double i = dot(intensity, vec3(tex_colour));
  // Use greyscale to as final colour
  // - ensure alpha is 1
  colour = vec4(i, i, i, 1.0f);
  colour += vec4(0.314f, 0.169f, -0.090f, 0.0f);
  // *********************************
}