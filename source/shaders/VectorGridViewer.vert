#version 460 core
layout (location = 0) in ivec3 vectorIndex;

out ivec3 id;

void main() {
	id = vectorIndex;
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
