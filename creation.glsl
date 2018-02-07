// Creation by Silexars
// https://www.shadertoy.com/view/XsXXDn
// Modified to use mouse input by Benedict Henshaw (2018-02-06)

#version 330

uniform vec2 resolution;
uniform vec2 mouse;
uniform float time;

out vec4 frag;

void main() {
    vec3 c;
    float l, z = time;
    for (int i = 0; i < 3; i++) {
        vec2 uv, p = gl_FragCoord.xy / resolution;
        uv = p;
        p.x -= mouse.x / resolution.x * 2.0;
        p.y -= mouse.y / resolution.y * 2.0 - 1.0;
        p.x *= resolution.x / resolution.y;
        z += 0.07;
        l = length(p);
        uv += p / l * (sin(z) + 1.0) * abs(sin(l * 9.0 - z * 2.0));
        c[i] = 0.01 / length(abs(mod(uv, 1.0) - 0.5));
    }
    frag = vec4(c / l, time);
}
