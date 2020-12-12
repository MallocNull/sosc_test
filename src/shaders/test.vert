#version 100
precision highp float;

attribute vec3 vertex;
attribute vec2 texuv;
attribute vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position =
        projection * view * model *
        vec4(vertex, 1.0);
}
