#version 460

uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat4 model_mat;
uniform vec4 uniform_color;
uniform float point_size;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in float vertex_reflectance;

out vec4 frag_color;

void main() {
    gl_Position = proj_mat * view_mat * model_mat * vec4(vertex_position, 1.0);
    gl_PointSize = point_size / gl_Position.w;
    frag_color = vec4(vertex_reflectance * uniform_color.xyz, 1.0f);
}
