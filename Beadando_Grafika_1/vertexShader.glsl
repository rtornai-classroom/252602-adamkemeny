#version 330 core

layout(location = 0) in vec2 position;
uniform vec2 circleCenter;
uniform float radius;
uniform bool isLine; 
uniform float lineCenterYLoc; 

void main() {
    vec2 transformedPosition;
    if (isLine) {
        transformedPosition.x = position.x;
        transformedPosition.y = position.y + lineCenterYLoc;
    } else {
        transformedPosition = (position * radius + circleCenter) / vec2(300.0, 300.0) - 1.0;
    }
    gl_Position = vec4(transformedPosition, 0.0, 1.0);
}
