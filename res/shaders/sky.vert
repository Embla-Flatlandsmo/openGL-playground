#version 430 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 textureCoordinates_in;


out layout(location=0) vec2 textureCoordinates_out;

void main()
{
    gl_Position = vec4(position.x, position.y, -1.0, 1.0); 
    textureCoordinates_out = textureCoordinates_in;
}  