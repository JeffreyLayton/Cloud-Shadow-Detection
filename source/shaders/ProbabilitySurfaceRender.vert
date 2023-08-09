#version 460 core
layout (location = 0) in ivec3 faceIndex;

out ivec3 id;

void main() {
	id = faceIndex;
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
