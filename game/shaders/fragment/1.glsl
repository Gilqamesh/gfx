#version 460

in vec3 color;
out vec4 ocolor;

void main() {
    ocolor = vec4(color, 1.0);
}
