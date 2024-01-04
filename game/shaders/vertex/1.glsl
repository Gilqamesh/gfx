#version 460

layout (location = 0) in vec2 pos;

out vec3 color;

vec2 positions[3] = {
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
};

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    // gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
    color = vec3(0.0, 0.0, 1.0);
}
