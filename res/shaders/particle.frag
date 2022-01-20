#version 430
out vec4 out_color;
//in range 0 - 1
in vec4 vert_color;

 
void main() {
	out_color = vert_color;
	// out_color = vec4(1.0, 0.0, 0.0, 1.0);
}