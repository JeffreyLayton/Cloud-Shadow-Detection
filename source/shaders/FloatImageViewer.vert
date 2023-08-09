#version 460 core
layout (location = 0) in vec3 posVert;
layout (location = 1) in vec2 uvVert;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec2 uvFrag;

void main() {
	uvFrag = uvVert;
	gl_Position = P * V * M * vec4(posVert, 1.0);
}
