#version 460

uniform sampler2D tex;
uniform float color_merge;
uniform vec4 uniform_color;

in vec2 frag_tex_coord;
in vec3 frag_normal;

out vec4 FragColor;

void main() {
    float light = (1.0 + dot(normalize(vec3(1.0, -1.0, 1.0)), -frag_normal)) * 0.5;
    vec4 color = texture(tex, frag_tex_coord);// color_merge * uniform_color + (1.0 - color_merge) * texture(tex, frag_tex_coord);
    FragColor = vec4(light * color.xyz, color.w);
}
