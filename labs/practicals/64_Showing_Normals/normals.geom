#version 440

// MVP transformation
uniform mat4 MVP;

// Layout of incoming data
layout(triangles) in;
// Layout of outgoing data
layout(line_strip, max_vertices = 6) out;

// Incoming normals for vertexs
layout(location = 0) in vec3 normal[];

void main() {
  // Calculate for each vertex
  for (int i = 0; i < 3; ++i) {
  
    // *********************************
    // Get vertex pos
    vec4 vertex_pos = gl_in[i].gl_Position;
    // Ensure normal is normalized
	normalize(normal[i]);
    // Output normal position for start of line
    // - remember to transform
	gl_Position = MVP * vertex_pos;
    // Emit
	EmitVertex();
    // Output position + normal for end of line
    // - remember to transform
	gl_Position = MVP * vec4((vec3(vertex_pos) + normal[i]), 1.0f);
    // Emit
	EmitVertex();
    // End the primitive
	EndPrimitive();
    // *********************************
  }
}