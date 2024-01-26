#version 460

layout (location = 0)
in vec2 pos;
// layout (location = 1) in vec3 color;

layout (location = 1)
in vec2 texture_coordinate;

uniform COMMON {
    mat4 mvp;
    float offset;
} vs_common;

/*out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};*/

out VS_OUT {
    //vec3 color;
    vec2 texture_coordinate;
    flat int texture_index;
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

    gl_Position = vs_common.mvp * p;
    //vs_common.offset;
    gl_PointSize = subroutine_uniform();
    //gl_PointSize = 50;
    vs_out.texture_coordinate = texture_coordinate;
    vs_out.texture_index = gl_InstanceID % 3;
/*
    if (texture_coordinate.x == 1.0f && texture_coordinate.y == 1.0f) {
        vs_out.color = vec3(1.0, 0.0, 0.0);
    } else if (texture_coordinate.x == 1.0f && texture_coordinate.y == 0.0f) {
        vs_out.color = vec3(0.0, 1.0, 0.0);
    } else if (texture_coordinate.x == 0.0f && texture_coordinate.y == 0.0f) {
        vs_out.color = vec3(0.0, 0.0, 1.0);
    } else {
        vs_out.color = vec3(1.0, 1.0, 1.0);
    }
*/
}
