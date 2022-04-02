#version 430
 out layout(location = 0) vec4 out_color;
// layout(location = 1) out vec4 out_depth;
//in range 0 - 1
in layout(location = 0) vec4 vert_color;
in layout(location = 1) vec4 frag_pos;
in layout(location = 2) vec3 normal_in;
in layout(location = 3) vec3 world_space_pos;

uniform vec3 camera_pos;

uniform vec3 light_direction = normalize(vec3(0.5, 0.5, 0.0));

uniform vec3 ambientColor = vec3(183, 237, 255)/255.0;
uniform vec3 diffuseColor = vec3(1.0);

uniform float fog_factor = 0.0075;
uniform vec3 fog_color = vec3(183, 237, 255)/255.0;

float fogIntensity(float d)
{
	return 1.0-exp2(-fog_factor*d);
}

void main() {
	// Lambert's cosine law
  	float lambertian = max(dot(normalize(normal_in), normalize(light_direction)), 0.0);
	vec4 color = vec4(1.0*lambertian*diffuseColor+0.2*ambientColor, 1.0);
	// float fog_length = (gl_DepthRange.diff * frag_pos.z + 
    //             gl_DepthRange.far + gl_DepthRange.near) / 2.0;
	// float fog_length = (gl_DepthRange.diff* frag_pos.z + gl_DepthRange.far + gl_DepthRange.near) / 2.0;
	float d = distance(camera_pos, world_space_pos);
	out_color = vec4(mix(color.xyz, fog_color, fogIntensity(d)), 1.0);
	// out_depth = vec4(frag_pos.xyz, 1.0);
	// out_depth = vec4(vec3(gl_FragDepth), 1.0);
	// out_depth = vec4(frag_pos.xy, (frag_pos.z-0.1)/10, 1.0);
	// out_color = vec4(1.0, 0.0, 0.0, 1.0);
	// out_depth = vec4(gl_FragCoord.zzz, 1.0);
}