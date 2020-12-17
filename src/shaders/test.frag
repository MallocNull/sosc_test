#version 100
precision mediump float;
varying vec3 f_normal;

const vec3 light_color = vec3(0.8, 0.8, 0.8);
const vec3 light = normalize(vec3(-1.0, -1.0, 1.0));

void main() {
    //gl_FragColor = vec4(coord.z, coord.z, coord.z, 1.0);

    //float xval = gl_FragCoord.x / 640.0;
    //float yval = gl_FragCoord.y / 480.0;
    vec3 ambient_color = vec3(0.2, 0.1, 0.1);
    float light_intensity = dot(light, normalize(f_normal));
    //vec3 color = f_normal;
    vec3 color = ambient_color + light_color * light_intensity;
    //vec3 color = vec3(light_intensity, light_intensity, light_intensity);

    gl_FragColor = vec4(color, 1.0);
}