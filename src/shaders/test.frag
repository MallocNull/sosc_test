#version 100
precision lowp float;

varying vec3 f_normal;
varying vec3 f_color;

const vec3 light_color = vec3(1.0, 1.0, 1.0);
const vec3 light_dir = normalize(vec3(-1.0, 2.0, -1.0));
const float ambient_strength = 0.1;

void main() {
    vec3 ambient = light_color * ambient_strength;
    vec3 norm = normalize(f_normal);

    float diff = max(0.0, dot(norm, light_dir));
    vec3 diffuse = diff * light_color;

    vec3 color = (ambient + diffuse) * f_color;
    gl_FragColor = vec4(color, 1.0);
    //gl_FragColor = vec4(f_normal, 1.0);
}

/*varying vec3 f_normal;
//varying vec3 f_pos;

const vec3 light_dir = normalize(vec3(1, 1, -1));
const vec3 object_color = vec3(.4157, .051, .6784);
const vec3 light_pos = vec3(3, 3, 3);
//const vec3 light = normalize(vec3(1.0, 1.0, 1.0));

void main() {
    //gl_FragColor = vec4(coord.z, coord.z, coord.z, 1.0);

    //float xval = gl_FragCoord.x / 640.0;
    //float yval = gl_FragCoord.y / 480.0;
    vec3 ambient = light_color * ambient_strength;
    vec3 norm = normalize(f_normal);
    //vec3 light_dir = normalize(light_pos - f_pos);
    float diff = max(0.0, dot(norm, light_dir));
    vec3 diffuse = diff * light_color;

    //vec3 color = (f_normal + vec3(1.0)) / vec3(2.0);
    //vec3 color = f_normal;
    //vec3 color = vec3(f_normal, f_normal, f_normal);

    vec3 color = (ambient + diffuse) * object_color;
    gl_FragColor = vec4(color, 1.0);
}*/