#version 440

#define M_PI 3.1415926535897932384626433832795

// The projection transformation
uniform mat4 P;

// Point size for the billboards
uniform float point_size;
// Eye position
uniform vec3 eye_pos;

// Incoming data
layout(points) in;
// Outgoing data
layout(triangle_strip, max_vertices = 52) out;

// Outgoing texture coordinate
layout(location = 0) out vec3 tex_coord;

void main() {
  // Incoming position
  vec4 position = gl_in[0].gl_Position;
  // Normal (faces camera)
  vec3 normal = vec3(0.0f, 0.0f, 1.0f);
  // Angle
  float delta_angle = (2.0 * M_PI) / 25.0f;

  // Create a disk to be environment mapped
  // *********************************************************
  // Process is:
  // 1. Start above the incoming position (top of the circle to be made)
  //    a. Multiply the offset by the point size for scaling
  // 2. Calculate position in camera space
  // 3. Set appropriate texture coord (environment mapping)
  // 4. Emit vertex
  // 5. Repeat for centre of the circle
  // 6. Repeat for the final vertex of this triangle which is
  //	delta_angle round from the top
  // 7. Complete the the triangle strip with the next vertex
  //	in the circle
  // 8. Save this vertex to be used as the start of the next
  //	part of the circle
  // 9. Emit primitive
  // *********************************************************

  // Carry vertex to be carried over to the next triangle strip
  // primitive
  // Top of triangle
  vec2 carry_vert = position.xy + vec2(0.0, 0.5) * point_size;

  for (int i = 1; i < 26; i++)
  {
	 // Vertex 1 is top
	 vec2 va = carry_vert;
	 gl_Position = P * vec4(va, position.zw);
	 tex_coord = normalize(reflect(vec3(va, 0.0f) - eye_pos, normal));
	 EmitVertex();
	 
	 // Vertex 2 is centre
	 vec2 vb = position.xy;
	 gl_Position = P * vec4(vb, position.zw);
	 tex_coord = normalize(reflect(vec3(vb, 0.0f) - eye_pos, normal));
	 EmitVertex();

	 // Vertex 3
	 vec2 vc = position.xy + vec2(0.5 * sin(i * delta_angle), 0.5 * cos(i * delta_angle)) * point_size;
	 gl_Position = P * vec4(vc, position.zw);
	 tex_coord = normalize(reflect(vec3(vc, 0.0f) - eye_pos, normal));
	 EmitVertex();

	 i = i + 1;

	 // Vertex 4
	 carry_vert = position.xy + vec2(0.5 * sin(i * delta_angle), 0.5 * cos(i * delta_angle)) * point_size;
	 gl_Position = P * vec4(carry_vert, position.zw);
	 tex_coord = normalize(reflect(vec3(carry_vert, 0.0f) - eye_pos, normal));
	 EmitVertex();

	 EndPrimitive();
  }
}