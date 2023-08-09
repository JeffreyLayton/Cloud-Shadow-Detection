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
uvec4 indexFrag(){
	uvec2 xy = xyFrag();
	uint base = 4 * (xy.y * inputImageWidth + xy.x);
	return uvec4(base + 0, base + 1, base + 2, base + 3);
}

out vec4 color;

void main() {
	uvec4 index = indexFrag();
	float red	= inputImage[index.r];
	float green = inputImage[index.g];
	float blue	= inputImage[index.b];
	float alpha = inputImage[index.a];
	color = vec4(red, green, blue, alpha);
}
