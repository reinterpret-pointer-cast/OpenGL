#version 410 core

in vec2 tex_coord;
in vec3 v_normal;
in vec3 v_pos;
in vec4 bw;

in vec4 vcolor;

in vec3 c_tangent;
in vec3 c_bitangent;
layout (location = 0) out vec4 color;

uniform sampler2D _t00; // aiTextureType_NONE
uniform sampler2D _t01; // aiTextureType_DIFFUSE
uniform sampler2D _t02; // aiTextureType_SPECULAR
uniform sampler2D _t03; // aiTextureType_AMBIENT
uniform sampler2D _t04; // aiTextureType_EMISSIVE
uniform sampler2D _t05; // aiTextureType_HEIGHT
uniform sampler2D _t06; // aiTextureType_NORMALS
uniform sampler2D _t07; // aiTextureType_SHININESS
uniform sampler2D _t08; // aiTextureType_OPACITY
uniform sampler2D _t09; // aiTextureType_DISPLACEMENT
uniform sampler2D _t10; // aiTextureType_LIGHTMAP
uniform sampler2D _t11; // aiTextureType_REFLECTION
uniform sampler2D _t12; // aiTextureType_BASE_COLOR
uniform sampler2D _t13; // aiTextureType_NORMAL_CAMERA
uniform sampler2D _t14; // aiTextureType_EMISSION_COLOR
uniform sampler2D _t15; // aiTextureType_METALNESS
uniform sampler2D _t16; // aiTextureType_DIFFUSE_ROUGHNESS
uniform sampler2D _t17; // aiTextureType_AMBIENT_OCCLUSION
uniform sampler2D _t18; // aiTextureType_SHEEN
uniform sampler2D _t19; // aiTextureType_CLEARCOAT
uniform sampler2D _t20; // aiTextureType_TRANSMISSION

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 view_p;
uniform float specular_strength = 0.5;
uniform float light_intensity;

#define AI_TEXTURE_TYPE_MAX 21
uniform vec4 material_colors[AI_TEXTURE_TYPE_MAX + 1];

void main() {
  ivec2 albedo_size = textureSize(_t12, 0);
  vec4 albedo;
  if (albedo_size.x > 0 && albedo_size.y > 0) {
      albedo = texture(_t12, tex_coord) * material_colors[12];
  } else {
      albedo = material_colors[12];
  }
  albedo *= vcolor;
  albedo.rgb = max(albedo.rgb, vec3(0.05));
  
  vec3 adjusted_light_color = light_color * light_intensity;
  vec3 ambient = 0.4 * adjusted_light_color;
  vec3 norm = normalize(v_normal);
  vec3 light_dir = normalize(light_position - v_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = diff * adjusted_light_color;
  vec3 view_dir = normalize(view_p - v_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
  vec3 specular = specular_strength * spec * adjusted_light_color;
  vec3 final_color = (ambient + diffuse + specular) * albedo.rgb;
  color = vec4(final_color, albedo.a);
}