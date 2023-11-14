#version 460

uniform sampler2D tex;
uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat4 model_mat;
uniform float point_size;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 vertex_texcoord;
layout (location = 2) in vec3 vertex_normal;

out vec4 frag_color;

void main() {
    gl_Position = proj_mat * view_mat * model_mat * vec4(vertex_position, 1.0);
    gl_PointSize = point_size / gl_Position.w;
    frag_color = texture(tex, vertex_texcoord);
}
