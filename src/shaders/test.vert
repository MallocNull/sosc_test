#version 100
precision highp float;

attribute vec3 vertex;
attribute vec2 texuv;
attribute vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec3 f_normal;

void main() {
    f_normal = normal;
    gl_Position =
        projection * view * model *
        vec4(vertex, 1.0);
}
