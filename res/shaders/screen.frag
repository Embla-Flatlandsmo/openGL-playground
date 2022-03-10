#version 430 core

out vec4 color;
  
in layout(location = 0) vec2 texCoords;

uniform layout(binding=0) sampler2D screenTexture;
uniform uint effect;

// Same as in screenQuad.hpp
#define NORMAL 0
#define CHROMATIC_ABERRATION 1
#define INVERT 2
#define BLUR 3
#define SHARPEN 4


const float offset = 1.0/300.0;

// Offsets for kernel operations (blur and sharpen)
const vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
);

vec3 applyFilter(float[9] kernel)
{
    vec3 color = vec3(0.0);
    vec3 sampledTexture[9];
    for (int i = 0; i < 9; i++) {
        sampledTexture[i] = vec3(texture(screenTexture, texCoords.st+offsets[i]));
    }
    for (int i = 0; i < 9; i++) {
        color += sampledTexture[i]*kernel[i];
    }
    return color;
}

void main()
{ 
    switch(effect) {
        case NORMAL:
            color = texture(screenTexture, texCoords);
            break;
        case CHROMATIC_ABERRATION:
            vec2 rOffset = vec2(0.002, 0.0);
            vec2 gOffset = vec2(0.0, 0.002);
            vec2 bOffset = vec2(0.0, 0.0);
            vec4 rValue = texture(screenTexture, texCoords - rOffset);  
            vec4 gValue = texture(screenTexture, texCoords - gOffset);
            vec4 bValue = texture(screenTexture, texCoords - bOffset);
            color = vec4(rValue.r, gValue.g, bValue.b, 1.0);

            break;  
        case INVERT:
            color = vec4(vec3(1.0 - texture(screenTexture, texCoords)), 1.0);
            break;
        case BLUR:
            float gaussian_kernel[9] = float[](
                1.0/16, 2.0/16, 1.0/16,
                2.0/16, 4.0/16, 2.0/16,
                1.0/16, 2.0/16, 1.0/16
            );
            color = vec4(applyFilter(gaussian_kernel), 1.0);
            break;
        case SHARPEN:
            float sharpen_kernel[9] = float[](
                -1, -1, -1,
                -1, 9, -1,
                -1, -1, -1
            );
            color = vec4(applyFilter(sharpen_kernel), 1.0);
            break;
        default:
            color = texture(screenTexture, texCoords);
            break;

    }

}