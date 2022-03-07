#version 430 core

uniform layout(location = 1) mat4 MVP;
layout (location = 0) in vec3 pos;
// out vec2 fragCoord;

void main() 
{
    gl_Position = vec4(pos.xy, 0.0f, 1.0f);
    // fragCoord = pos.xy;
}