#version 460 core
// -------------------------------------------------------
// --------------------- IO ------------------------------
in vec3 normal;
in vec3 pos;
in vec4 col;
out vec4 color;
// -------------------------------------------------------
// -------------------------------------------------------
// --------------------- Uniforms ------------------------
uniform vec3 light_pos;
uniform vec3 camera_pos;
// -------------------------------------------------------

void main() {
	vec3 V = normalize(camera_pos - pos);
	vec3 L = normalize(light_pos  - pos);
	vec3 N = normalize(normal);

	vec3 AMB = vec3(0.05)									         ;
	vec3 DIF = vec3(0.70) * max(dot(L,N), 0.0)				         ;
	vec3 SPE = vec3(0.40) * pow(max(dot(reflect(-L,N),V), 0.0), 64.0);

	color = col * vec4(AMB + DIF + SPE, 1.0);
}
