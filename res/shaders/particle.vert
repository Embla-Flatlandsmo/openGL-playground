#version 430 core

uniform layout(location = 1) mat4 VP;
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 col;

out vec4 vert_color;

void main() 
{
	gl_Position = VP*aPos;
	vert_color = col;
}