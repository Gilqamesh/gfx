#version 460

layout (binding = 0)
uniform sampler2DArray texture_sampler;

out vec4 ocolor;

// gl_FragCoord

in VS_OUT {
    //vec3 color;
    vec2 texture_coordinate;
    flat int texture_index;
} fs_in;

void main() {
    //ocolor = vec4(fs_in.color, 1.0);
    //int texture_size = textureSize(texture_sampler);
    //int texel_index = int(cos(gl_FragCoord.x) * gl_FragCoord.y) % texture_size;
    //ocolor = texelFetch(texture_sampler, texel_index).rgba;
    ocolor = texture(texture_sampler, vec3(fs_in.texture_coordinate, float(fs_in.texture_index)));
    //ocolor = texture(texture_sampler, fs_in.texture_coordinate);
}
