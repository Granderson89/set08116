#version 440

// Main textures
uniform sampler2D tex[4];
// Blend map
uniform sampler2D blend_map;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;
// Outgoing fragment colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample the two main textures
  vec4 colour1 = texture(tex[0], tex_coord);
  vec4 colour2 = texture(tex[1], tex_coord);
  vec4 colour3 = texture(tex[2], tex_coord);
  vec4 colour4 = texture(tex[3], tex_coord);
  // Sample the blend texture
  vec4 blend = texture(blend_map, tex_coord);
  // Mix the main samples using r component from 
  // blend value
  vec4 mix1 = mix(colour1, colour2, blend.r);
  vec4 mix2 = mix(colour3, colour4, blend.g);
  colour = mix(mix1, mix2, blend.b);
  // *********************************
}