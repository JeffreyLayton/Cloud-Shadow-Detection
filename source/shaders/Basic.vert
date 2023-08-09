#version 460 core
layout (location = 0) in vec3 posVert;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
void main() {gl_Position = P * V * M * vec4(posVert, 1.0);}
