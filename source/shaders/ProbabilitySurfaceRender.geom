#version 460 core
// -------------------------------------------------------
// --------------------- IO ------------------------------
	layout(points) in;
	in ivec3 id[];

	layout(triangle_strip, max_vertices = 3) out;
	out vec3 normal, pos;
	out vec4 col;
// -------------------------------------------------------
// -------------------------------------------------------
// --------------------- Buffers -------------------------
	struct Vertex{float x, y, z;};
	uniform ivec2 root;
	uniform ivec2 render_dim;
	uniform ivec2 total_dim;
	layout(binding = 0, std430) buffer vertexBuffer{Vertex vertices[];};
// -------------------------------------------------------
// -------------------------------------------------------
// --------------------- MVP -----------------------------
	uniform mat4 M, V, P;
// -------------------------------------------------------
    uniform vec4 cols[3];

vec3 getVertex(ivec2 vertex_id){
	Vertex vert = vertices[(vertex_id.y - root.y) + render_dim.y * (vertex_id.x - root.x)];
	return vec3(vert.x, vert.y, vert.z);
}

vec4 getColour(ivec2 v_id){
	bool out_x = v_id.x < 0 || v_id.x >= total_dim.x;
	bool out_y = v_id.y < 0 || v_id.y >= total_dim.y;
	return cols[(out_x && out_y) ? 2 : (out_x || out_y) ? 1 : 0];
}

struct Tri{vec3 p0, p1, p2, n; vec4 c0, c1, c2;};
Tri getTri(ivec3 tri_id){
	Tri t;
	t.p0 = getVertex(tri_id.xy);
	t.c0 = getColour(tri_id.xy);
	if(tri_id.z < 1){
		t.p1 = getVertex(tri_id.xy + ivec2(1,0));
		t.c1 = getColour(tri_id.xy + ivec2(1,0));
		t.p2 = getVertex(tri_id.xy + ivec2(1,1));
		t.c2 = getColour(tri_id.xy + ivec2(1,1));
	}else{
		t.p1 = getVertex(tri_id.xy + ivec2(1,1));
		t.c1 = getColour(tri_id.xy + ivec2(1,1));
		t.p2 = getVertex(tri_id.xy + ivec2(0,1));
		t.c2 = getColour(tri_id.xy + ivec2(0,1));
	}
	t.n = normalize(cross(t.p1 - t.p0, t.p2 - t.p0));
	return t;
}

void emit(vec3 p, vec3 n, vec4 c){
	normal = n;
	pos = p;
	col = c;
	gl_Position = P * V * vec4(p, 1.0);
	EmitVertex();
}


void main(){
	ivec3 face = id[0];
	if(face.x <  root.x                    || face.y <  root.y                   ) return;
	if(face.x >= root.x + render_dim.x - 1 || face.y >= root.y + render_dim.y - 1) return;

	Tri t = getTri(face);

	t.p0 = vec3(M * vec4(t.p0, 1.0));
	t.p1 = vec3(M * vec4(t.p1, 1.0));
	t.p2 = vec3(M * vec4(t.p2, 1.0));
	t.n = normalize(vec3(inverse(transpose(M)) * vec4(t.n, 0.0)));

	emit(t.p0, t.n, t.c0);
	emit(t.p1, t.n, t.c1);
	emit(t.p2, t.n, t.c2);
	EndPrimitive();
}
