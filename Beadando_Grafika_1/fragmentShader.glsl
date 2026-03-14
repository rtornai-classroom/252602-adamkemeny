#version 330 core

out vec4 fragColor;
uniform vec2 circleCenter;
uniform float radius;
uniform bool isLine;
uniform int colorSwap;

void main() {
    if (isLine) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    } else {
        float dist = distance(gl_FragCoord.xy, circleCenter);
        if (dist < radius) {
            float t = dist / radius;

            vec3 centerColor = vec3(1.0, 0.0, 0.0);
            vec3 borderColor = vec3(0.0, 1.0, 0.0);

            if (colorSwap == 1) {
                centerColor = vec3(0.0, 1.0, 0.0);
                borderColor = vec3(1.0, 0.0, 0.0);
            }

            fragColor = vec4(mix(centerColor, borderColor, t), 1.0);
        } else {
            discard;
        }
    }
}
