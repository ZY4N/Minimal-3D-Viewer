#version 460

uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat4 model_mat;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 vertex_texcoord;
layout (location = 2) in vec3 vertex_normal;

out vec2 frag_tex_coord;
out vec3 frag_normal;

void main() {
    gl_Position = proj_mat * view_mat * model_mat * vec4(vertex_position, 1.0);
    frag_tex_coord = vertex_texcoord;
    frag_normal = vertex_normal;
}
