#version 460 core
in vec2 uvFrag;

uniform uint inputImageSize;
uniform uint inputImageWidth;
uniform uint inputImageHeight;
layout(binding = 0, std430) buffer inputImageSSBO{
	float inputImage[];
};
uvec2 xyFrag(){
	return uvec2(uint(uvFrag.x * float(inputImageWidth)), uint((1. - uvFrag.y) * float(inputImageHeight)));
}
uint indexFrag(){
	uvec2 xy = xyFrag();
	return xy.y * inputImageWidth + xy.x;
}

out vec4 color;

void main() {
	float gray = inputImage[indexFrag()];
	color = vec4(gray, gray, gray, 1.0);
}
