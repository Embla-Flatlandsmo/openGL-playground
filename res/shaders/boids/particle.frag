#version 430
layout(location = 0) out vec4 out_color;
// layout(location = 1) out vec4 out_depth;
//in range 0 - 1
in vec4 vert_color;
// in vec4 frag_pos;
 
void main() {
	out_color = vert_color;
	// out_depth = vec4(frag_pos.xyz, 1.0);
	// out_depth = vec4(vec3(gl_FragDepth), 1.0);
	// out_depth = vec4(frag_pos.xy, (frag_pos.z-0.1)/10, 1.0);
	// out_color = vec4(1.0, 0.0, 0.0, 1.0);
	// out_depth = vec4(gl_FragCoord.zzz, 1.0);
}