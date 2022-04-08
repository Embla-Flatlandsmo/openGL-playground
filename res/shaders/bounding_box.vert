#version 430 core

uniform layout(location = 1) mat4 MVP;
layout (location = 0) in vec3 pos;



void main() 
{
    gl_Position = MVP * vec4(pos, 1.0f);
}