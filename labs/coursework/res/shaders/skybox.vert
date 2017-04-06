#version 440

// MVP transformation matrix
uniform mat4 MVP;
// MV transformation
uniform mat4 MV;

// Incoming position
layout (location = 0) in vec3 position;

// Outgoing 3D texture coordinate
layout (location = 0) out vec3 tex_coord;
// Camera space position
layout(location = 1) out vec4 CS_position;

void main()
{
	// Calculate screen space position
	gl_Position = MVP * vec4(position, 1.0);
	  // *********************************
  // Calculate camera space coordinate
  // *********************************
  CS_position = MV * vec4(position, 1.0);
	// *******************************
	// Set outgoing texture coordinate
	// *******************************
	tex_coord = position;
}