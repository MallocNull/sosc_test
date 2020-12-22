#version 100
precision highp float;

attribute vec3 vertex;
attribute vec2 texuv;
attribute vec3 normal;
attribute vec3 color;

/*layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 texuv;
layout(location = 2) in vec3 normal;*/

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec3 f_normal;
varying vec3 f_color;

void main() {
    //f_normal = normal;
    //f_pos = vertex;
    /*vec3 norm = normalize(normal);
    float intensity = max(dot(norm, light_dir), 0.0);
    f_color = intensity * vec3(.4157, .051, .6784);*/
    f_normal = normalize(normal);
    f_color = color;

    gl_Position =
        projection * view * model *
        vec4(vertex, 1.0);
}
