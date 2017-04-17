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

  // *********************************************************
  // Process is:
  // 1. Calculate position in camera space (position + offset)
  //     a. Multiply the offset by the point size for scaling
  // 2. Transform into camera space for gl_Position
  // 3. Set appropriate texture coordinate
  // 4. Emit
  // *********************************************************

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
  /*
  // Vertex 1 is vertex 4 from previous
  gl_Position = P * vec4(vd, position.zw);
  tex_coord = normalize(reflect(vec3(vd, 0.0f) - eye_pos, normal));
  EmitVertex();

  // Vertex 2 is centre
  gl_Position = P * vec4(vb, position.zw);
  tex_coord = normalize(reflect(vec3(vb, 0.0f) - eye_pos, normal));
  EmitVertex();

  // Vertex 3
  vec2 ve = position.xy + vec2(0.5 * sin(3.0 * delta_angle), 0.5 * cos(3.0 * delta_angle)) * point_size;
  gl_Position = P * vec4(ve, position.zw);
  tex_coord = normalize(reflect(vec3(ve, 0.0f) - eye_pos, normal));
  EmitVertex();

  // Vertex 4
  vec2 vf = position.xy + vec2(0.5 * sin(4.0 * delta_angle), 0.5 * cos(4.0 * delta_angle)) * point_size;
  gl_Position = P * vec4(vf, position.zw);
  tex_coord = normalize(reflect(vec3(vf, 0.0f) - eye_pos, normal));
  EmitVertex();

  EndPrimitive();

  
  // Vertex 5 is left
  vec2 ve = position.xy + vec2(-0.5, 0.0) * point_size;
  gl_Position = P * vec4(ve, position.zw);
  tex_coord = normalize(reflect(vec3(ve, 0.0f) - eye_pos, normal));
  EmitVertex();

  // Vertex 6 is centre
  gl_Position = P * vec4(vb, position.zw);
  tex_coord = normalize(reflect(vec3(vb, 0.0f) - eye_pos, normal));
  EmitVertex();

  // Vertex 7 is top
  gl_Position = P * vec4(va, position.zw);
  tex_coord = normalize(reflect(vec3(va, 0.0f) - eye_pos, normal));
  EmitVertex();
  */
  // End Primitive
  //EndPrimitive();
}