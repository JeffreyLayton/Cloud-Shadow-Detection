#version 460 core
// -------------------------------------------------------
// --------------------- IO ------------------------------
	layout(points) in;
	in ivec3 id[];

	layout(line_strip, max_vertices = 2) out;
	out vec4 col;
// -------------------------------------------------------
// -------------------------------------------------------
// --------------------- Buffers -------------------------
uniform uint inputImageSize;
uniform uint inputImageWidth;
uniform uint inputImageHeight;
layout(binding = 0, std430) buffer inputImageSSBO{
	float inputImage[];
};
uvec3 indexFrag(vec2 xy){
    uvec2 id = uvec2(xy);
	uint base = 3 * (id.y * inputImageWidth + id.x);
	return uvec3(base + 0, base + 1, base + 2);
}
// -------------------------------------------------------
// -------------------------------------------------------
// --------------------- MVP -----------------------------
	uniform mat4 M, V, P;
// -------------------------------------------------------
    uniform vec4 cols[2];
	uniform ivec2 divs;
	uniform vec2 vec_lengths;
	uniform vec3 ave_dir;
	uniform float forward_dir_weight;
	uniform int remove_dc_dir;


void emit(vec3 p, vec4 c){
	col = c;
	gl_Position = P * V * M * vec4(p, 1.);
	EmitVertex();
}


void main(){
	ivec3 vector = id[0];
	if(vector.x < 0 || vector.y < 0) return;
	if(vector.x >= divs.x || vector.y >= divs.y) return;

	vec2 vec_uv = (vec2(vector) + vec2(.5)) / vec2(divs);
	vec3 pos_image_space = {vec_uv.x * float(inputImageWidth), (1. - vec_uv.y) * float(inputImageHeight), 0.};
	uvec3 indices        = indexFrag(pos_image_space.xy);
	vec3 d_image_space   = vec3(inputImage[indices.r], -inputImage[indices.g], inputImage[indices.b]);

	if(remove_dc_dir != 0)
		d_image_space = normalize(d_image_space - ave_dir + vec3(0.,0.,forward_dir_weight));

	emit(pos_image_space - d_image_space * vec_lengths[0], cols[0]);
	emit(pos_image_space + d_image_space * vec_lengths[1], cols[1]);
	EndPrimitive();
}
