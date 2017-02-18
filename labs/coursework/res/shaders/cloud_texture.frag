#version 440

// Sampler used to get texture colour
uniform sampler2D tex;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;
// Outgoing colour
layout(location = 0) out vec4 out_colour;

void main() {
  // *********************************
  float transparency_factor = 0.3f;
  out_colour = texture(tex, tex_coord);
  if (out_colour.r < transparency_factor && 
      out_colour.g < transparency_factor && 
	  out_colour.b < transparency_factor)
  {
     out_colour.a = 0.0f;
  }
  else
  {
     out_colour.a = 1.0f;
  }
  // *********************************
}