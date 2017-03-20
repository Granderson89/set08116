#version 430 core

// Incoming frame data
uniform sampler2D tex;

// 1.0f / screen width
uniform float inverse_width;
// 1.0f / screen height
uniform float inverse_height;

// Surrounding pixels to sample and their scale
const vec4 samples[4] = vec4[4](vec4(-1.0, 0.0, 0.0, 0.25), vec4(1.0, 0.0, 0.0, 0.25), vec4(0.0, 1.0, 0.0, 0.25),
                                vec4(0.0, -1.0, 0.0, 0.25));
// Edge detection
//const vec4 samples[6] = vec4[6](vec4(-1.0, 1.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 2.0), vec4(1.0, 1.0, 0.0, 1.0),
//                                vec4(-1.0, -1.0, 0.0, 1.0), vec4(0.0, -1.0, 0.0, -2.0), vec4(1.0, -1.0, 0.0, -1.0));
								
// Sharpening
// const vec4 samples[5] = vec4[5](vec4(0.0, 1.0, 0.0, -2.0 / 3.0), vec4(-1.0, 0.0, 0.0, -2.0 / 3.0), vec4(0.0, 0.0, 0.0, 11.0 / 3.0),
//								vec4(1.0, 0.0, 0.0, -2.0 / 3.0), vec4(0.0, -1.0, 0.0, -2.0 / 3.0));

// Gaussian part one
//const vec4 gaussian_one[7] = vec4[7](vec4(-3.0, 0.0, 0.0, 0.015625), vec4(-2.0, 0.0, 0.0, 0.09375), vec4(-1.0, 0.0, 0.0, 0.234375),
//									 vec4(0.0, 0.0, 0.0, 0.3125), vec4(1.0, 0.0, 0.0, 0.234375), vec4(2.0, 0.0, 0.0, 0.09375), vec4(3.0, 0.0, 0.0, 0.015625));
// Gaussian part two
//const vec4 gaussian_two[7] = vec4[7](vec4(0.0, -3.0, 0.0, 0.015625), vec4(0.0, -2.0, 0.0, 0.09375), vec4(0.0, -1.0, 0.0, 0.234375),
//									 vec4(0.0, 0.0, 0.0, 0.3125), vec4(0.0, 1.0, 0.0, 0.234375), vec4(0.0, 2.0, 0.0, 0.09375), vec4(0.0, 3.0, 0.0, 0.015625));

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Start with colour as black
  colour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  // Loop through each sample vector
  for (int i = 0; i < 4; i++)
  {
    // Calculate tex coord to sample
	vec2 uv = tex_coord + vec2(samples[i].x * inverse_width, samples[i].y * inverse_height);
    // Sample the texture and scale appropriately
    // - scale factor stored in w component
	colour += texture(tex, uv) * samples[i].w;
  }
  /*
  for (int i = 0; i < 7; i++)
  {
    // Calculate tex coord to sample
	vec2 uv = tex_coord + vec2(gaussian_one[i].x * inverse_width, gaussian_one[i].y * inverse_height);
    // Sample the texture and scale appropriately
    // - scale factor stored in w component
	colour += texture(tex, uv) * gaussian_one[i].w;
  }
  for (int i = 0; i < 7; i++)
  {
    // Calculate tex coord to sample
	vec2 uv = tex_coord + vec2(gaussian_two[i].x * inverse_width, gaussian_two[i].y * inverse_height);
    // Sample the texture and scale appropriately
    // - scale factor stored in w component
	colour += texture(tex, uv) * gaussian_two[i].w;
  }
  */
  // Ensure alpha is 1.0
  colour.a = 1.0f;
  // *********************************
}