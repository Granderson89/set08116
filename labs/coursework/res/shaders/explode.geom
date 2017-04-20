#version 440

// Explode factor (how much to deform the sun)
uniform float explode_factor;
// Peak factor (how much more to deform the active area)
uniform float peak_factor;
// Active position on sun surface
uniform vec3 sun_activity;

// Layout of incoming data
layout(triangles) in;
// Layout of outgoing data
layout(triangle_strip, max_vertices = 6) out;

// Incoming vertex position
layout (location = 0) in vec3 vertex_position_in[];
// Incoming texture coordinate
layout (location = 1) in vec2 tex_coord_out_in[];
// Incoming transformed normal
layout(location = 2) in vec3 transformed_normal_in[];
// Incoming tangent
layout(location = 3) in vec3 tangent_out_in[];
// Incoming binormal
layout(location = 4) in vec3 binormal_out_in[];
// Incoming position in light space
layout (location = 5) in vec4 light_space_pos_in[];

// Outgoing position
layout(location = 0) out vec3 vertex_position;
// Outgoing texture coordinate
layout(location = 1) out vec2 tex_coord_out;
// Outgoing normal
layout(location = 2) out vec3 transformed_normal;
// Outgoing tangent
layout(location = 3) out vec3 tangent_out;
// Outgoing binormal
layout(location = 4) out vec3 binormal_out;
// Outgoing light space position
layout(location = 5) out vec4 light_space_pos;


void main() {

  // Calculate Face Normal
  vec3 P0 = gl_in[0].gl_Position.xyz;
  vec3 P1 = gl_in[1].gl_Position.xyz;
  vec3 P2 = gl_in[2].gl_Position.xyz;
  vec3 V0 = P0 - P1;
  vec3 V1 = P2 - P1;
  vec3 face_normal = normalize(cross(V1, V0));

  // Deforming vertices at poles results in large spikes.
  // Avoid deforming these
  if (vertex_position_in[0].y >= 8.0f || vertex_position_in[0].y <= -8.0f)
  {
    gl_Position = gl_in[0].gl_Position;
    vertex_position = vertex_position_in[0];
  }
  // For all others, move the first vertex out. How far it moves from the surface
  // depends on whether or not it is in an active area. If so, it moves out further.
  else
  {
	if (distance(vertex_position_in[0], sun_activity) <= 2.0f)
		{
			gl_Position = gl_in[0].gl_Position - vec4(face_normal, 0.0f) * explode_factor * peak_factor;
			vertex_position = vertex_position_in[0] - face_normal * explode_factor * peak_factor;
		}
		else
		{
			gl_Position = gl_in[0].gl_Position - vec4(face_normal, 0.0f) * explode_factor;
			vertex_position = vertex_position_in[0] - face_normal * explode_factor;
		}
  }
  tex_coord_out = tex_coord_out_in[0];
  transformed_normal = transformed_normal_in[0];
  tangent_out = tangent_out_in[0];
  binormal_out = binormal_out_in[0];
  light_space_pos = light_space_pos_in[0];
  // Emit Vertex
  EmitVertex();

  // All other vertices remain the same
  gl_Position = gl_in[1].gl_Position;
  vertex_position = vertex_position_in[1];
  tex_coord_out = tex_coord_out_in[1];
  transformed_normal = transformed_normal_in[1];
  tangent_out = tangent_out_in[1];
  binormal_out = binormal_out_in[1];
  light_space_pos = light_space_pos_in[1];
  // Emit Vertex
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  vertex_position = vertex_position_in[2];
  tex_coord_out = tex_coord_out_in[2];
  transformed_normal = transformed_normal_in[2];
  tangent_out = tangent_out_in[2];
  binormal_out = binormal_out_in[2];
  light_space_pos = light_space_pos_in[2];
  // Emit Vertex
  EmitVertex();

  // Duplicate the triangle so no gaps appear in the render
  gl_Position = gl_in[0].gl_Position;
  vertex_position = vertex_position_in[0];
  tex_coord_out = tex_coord_out_in[0];
  transformed_normal = transformed_normal_in[0];
  tangent_out = tangent_out_in[0];
  binormal_out = binormal_out_in[0];
  light_space_pos = light_space_pos_in[0];
  // Emit Vertex
  EmitVertex();
  gl_Position = gl_in[1].gl_Position;
  vertex_position = vertex_position_in[1];
  tex_coord_out = tex_coord_out_in[1];
  transformed_normal = transformed_normal_in[1];
  tangent_out = tangent_out_in[1];
  binormal_out = binormal_out_in[1];
  light_space_pos = light_space_pos_in[1];
  // Emit Vertex
  EmitVertex();
  gl_Position = gl_in[2].gl_Position;
  vertex_position = vertex_position_in[2];
  tex_coord_out = tex_coord_out_in[2];
  transformed_normal = transformed_normal_in[2];
  tangent_out = tangent_out_in[2];
  binormal_out = binormal_out_in[2];
  light_space_pos = light_space_pos_in[2];
  // Emit Vertex
  EmitVertex();
  
  // End Primitive
  EndPrimitive();
  
}