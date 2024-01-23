#version 460

layout (location = 0) in vec2 pos;
// layout (location = 1) in vec3 color;
layout (location = 1) in vec2 texture_coordinate;

/*out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};*/

out VS_OUT {
    // vec3 color;
    vec2 texture_coordinate;
} vs_out;

// gl_VertexID
// gl_InstanceID
// gl_PointSize

subroutine float subroutine__get_point_size(void);

layout (index = 1)
subroutine (subroutine__get_point_size)
float point_size_1(void) {
    return 5;
}

layout (index = 2)
subroutine (subroutine__get_point_size)
float point_size_2(void) {
    return 50;
}

subroutine uniform subroutine__get_point_size subroutine_uniform;

void main() {
    float xoffset = -0.5f;
    float xstretch = 0.3f;
    vec4 p = vec4(
        pos.x + xoffset + gl_InstanceID * xstretch, pos.y,
        0.0, 1.0
    );
    gl_Position = p;
    gl_PointSize = subroutine_uniform();
    //gl_PointSize = 50;
    vs_out.texture_coordinate = texture_coordinate;
}
