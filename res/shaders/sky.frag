#version 330 core
out vec4 FragColor;

uniform vec3 skyColorBottom = vec3(183, 237, 255)/255.0;

// uniform vec3 skyColorBottom = vec3(0.0,0.74902, 1.0);
uniform vec3 skyColorTop = vec3(0.0,0.74902, 1.0);

uniform vec3 lightDirection = normalize(vec3(0.5, 0.5, 0.0));

uniform vec2 resolution = vec2(1366, 768);

uniform mat4 inv_proj;
uniform mat4 inv_view;


// uniform vec3 sun_direction;
// uniform float sun_distance = 100.0;

// uniform mat4 view_mat;
// uniform mat4 inv_view;
// uniform mat4 inv_proj;
uniform vec4 viewport = vec4(0.0, 0.0, 1366.0, 768.0);

// // const vec3 skyColour = 0.6 * vec3(0.09, 0.33, 0.81);
// const vec3 skyColour = 0.6*vec3(0.0,0.74902, 1.0);

// out vec4 color;

#define SUN_DIR lightDirection

// Finds fragment coordinate fragCoord.x = [0,windowWidth], fragCoord.y=[0,windowHeight]
// and converts it to normalized device coordinates [-1, 1]
vec3 fragToClipSpace(uvec2 fragCoord)
{
  vec2 ndc = 2.0*vec2(fragCoord.xy-viewport.xy)/viewport.zw - 1.0;
  return vec3(ndc, 1.0);
}

vec3 getSun(const vec3 d, float powExp){
	float sun = clamp( dot(SUN_DIR,d), 0.0, 1.0 );
	vec3 col = 0.8*vec3(1.0,.6,0.1)*pow( sun, powExp );
	return col;
}

vec3 getSkyColour(vec3 rayDir)
{
    vec3 sky = mix(skyColorBottom, skyColorTop, clamp(1-exp(-2*normalize(rayDir).y), 0.0, 1.0));

    return sky + getSun(rayDir, 350.0);
}

void main()
{    
	uvec2 fragCoord = uvec2(gl_FragCoord.xy);

    vec4 ray_dir_ndc = vec4(fragToClipSpace(fragCoord), 1.0);
    vec4 ray_dir_view = inv_proj*ray_dir_ndc;

        // I don't know why this works, but it does
    ray_dir_view = vec4(ray_dir_view.xy, -1.0, 0.0);
    vec4 ray_dir_world = (inv_view*ray_dir_view);

	FragColor = vec4(getSkyColour(normalize(ray_dir_world.xyz)), 1.0);
}











// #version 430 core


// uniform vec3 sun_direction;
// uniform float sun_distance = 100.0;

// uniform mat4 view_mat;
// uniform mat4 inv_view;
// uniform mat4 inv_proj;
// uniform vec4 viewport = vec4(0.0, 0.0, 1366.0, 768.0);

// // const vec3 skyColour = 0.6 * vec3(0.09, 0.33, 0.81);
// const vec3 skyColour = 0.6*vec3(0.0,0.74902, 1.0);

// out vec4 color;

// vec3 rayDirection(float fieldOfView, vec2 fragCoord) {
//     vec2 xy = fragCoord - viewport.zw / 2.0;
//     float z = (0.5 * viewport.z) / tan(radians(80.0) / 2.0);
//     return normalize(vec3(xy, -z));
// }

// //Darken sky when looking up.
// vec3 getSkyColour(vec3 rayDir){
//     return mix(skyColour, 0.5*skyColour, rayDir.y);
// }

// void main()
// {
//     // vec4 ray_dir_ndc = vec4(gl_FragCoord.xy, 1.0 , 1.0);
//     // vec4 ray_dir_view = inv_proj*ray_dir_ndc;

//     // // I don't know why this works, but it does
//     // ray_dir_view = vec4(ray_dir_view.xy, -1.0, 0.0);
//     // vec4 ray_dir_world = (inv_view*ray_dir_view);
//     vec4 rayDir = vec4(rayDirection(80.0, gl_FragCoord.xy), 1.0);
//     rayDir = normalize(view_mat * rayDir);
//     color = vec4(getSkyColour(rayDir.xyz), 1.0);
// }