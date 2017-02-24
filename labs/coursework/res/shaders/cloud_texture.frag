#version 440

// Point light information
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

// Material information
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Point light for the scene
uniform point_light point;
/*
// Spot light for the scene
uniform spot_light spot;
*/
// Material for the object
uniform material mat;
// Eye position
uniform vec3 eye_pos;

// Sampler used to get texture colour
uniform sampler2D tex;

// Incoming texture coordinate
layout(location = 0) in vec3 vertex_position;
// Incoming normal
layout(location = 1) in vec3 transformed_normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord_out;


// Outgoing colour
layout(location = 0) out vec4 out_colour;

// Point light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 vertex_position, in vec3 transformed_normal, in vec3 view_dir,
                     in vec4 tex_colour) {
  // *********************************
  // Get distance between point light and vertex
  float d = distance(point.position, vertex_position);
  // Calculate attenuation factor
  float att = 1.0f / (point.constant + point.linear * d + point.quadratic * (d * d));
  // Calculate light colour
  vec4 light_colour = att * point.light_colour;
  // Calculate light dir
  vec3 light_dir = normalize(point.position - vertex_position);
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient
  // Calculate diffuse component
  vec4 diffuse = max(dot(transformed_normal, light_dir), 0.0) * (mat.diffuse_reflection * light_colour);
  // Calculate half vector
  vec3 half_vector = normalize(light_dir + view_dir);
  // Calculate specular component
  float ks = pow(max(dot(half_vector, transformed_normal), 0.0), mat.shininess);
  vec4 specular = ks * (mat.specular_reflection * light_colour);
  // Calculate primary colour component
  vec4 primary = mat.emissive + diffuse;
  // Calculate final colour - remember alpha
  vec4 final_colour = primary * tex_colour + specular;
  final_colour.a = 1.0f;
  // *********************************
  return final_colour;
}


void main() {
  // *********************************
  out_colour = vec4(0.0, 0.0, 0.0, 1.0);
  // *********************************
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - vertex_position);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord_out);
  out_colour = calculate_point(point, mat, vertex_position, transformed_normal, view_dir,
                     tex_colour);

  float transparency_factor = 0.3f;
  if (out_colour.r < transparency_factor && 
      out_colour.g < transparency_factor && 
	  out_colour.b < transparency_factor)
  {
     out_colour.a = 0.0f;
  }
  // *********************************
}