#version 440

// Model view projection matrix
uniform mat4 MVP;
// The transformation matrix
uniform mat4 M;
// The normal matrix
uniform mat3 N;

// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming texture coordinate
layout (location = 10) in vec2 tex_coord_in;

// Outgoing vertex position
layout (location = 0) out vec3 vertex_position;
// Outgoing transformed normal
layout(location = 1) out vec3 transformed_normal;
// Outgoing texture coordinate
layout (location = 2) out vec2 tex_coord_out;


void main()
{
	// Calculate screen position of vertex
	gl_Position = MVP * vec4(position, 1.0);
	// Output other values to fragment shader
	transformed_normal = -1.0f * N * normal;
	// Calculate world position of vertex
	vertex_position = vec3(M * vec4(position, 1.0f));
	tex_coord_out = tex_coord_in;
}