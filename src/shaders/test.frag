#version 100
precision mediump float;

//varying vec3 coord;

void main() {
    //gl_FragColor = vec4(coord.z, coord.z, coord.z, 1.0);

    float xval = gl_FragCoord.x / 640.0;
    float yval = gl_FragCoord.y / 480.0;
    gl_FragColor = vec4(xval, yval, 1.0, 1.0);
}