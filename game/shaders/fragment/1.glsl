#version 460

layout (binding = 0) uniform sampler2D texture_sampler;

out vec4 ocolor;

// gl_FragCoord

in VS_OUT {
    // vec3 color;
    vec2 texture_coordinate;
} vs_in;

void main() {
    ocolor = texture(texture_sampler, vs_in.texture_coordinate);
}
