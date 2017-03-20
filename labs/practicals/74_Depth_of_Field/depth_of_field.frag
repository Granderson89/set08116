#version 410

// Previous pass capture
uniform sampler2D tex;
// Initial render capture
uniform sampler2D sharp;
// Initial render depth capture
uniform sampler2D depth;

// The range value of the dof
uniform float range;
// The focus distance of the dof
uniform float focus;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample sharp texture
  vec4 sharp_sample = texture(sharp, tex_coord);
  // Sample blur texture
  vec4 blur_sample = texture(tex, tex_coord);
  // Calculate distance from the camera - based on depth sample
  vec4 depth_sample = texture(depth, tex_coord);
  float dist = depth_sample.z;
  // Mix samples together based on distance
  colour = mix(sharp_sample, blur_sample, dist);
  // Ensure alpha is 1.0
  colour.a = 1.0f;
  // *********************************
}