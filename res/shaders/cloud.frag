#version 430 core

in vec4 gl_FragCoord;
out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform mat4 inv_vp;
uniform mat4 inv_view;
uniform mat4 inv_proj;
uniform vec4 viewport;
uniform vec3 cam_pos;

// uniform vec3 volExtentMin;
// uniform vec3 volExtentMax;

uniform sampler3D perlin;
uniform sampler3D worley32;

void main(void)
{
	// fragColor = vec4(1.0);

	vec4 ndcPos;
	ndcPos.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
	ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
		(gl_DepthRange.far - gl_DepthRange.near);
	ndcPos.w = 1.0;

	vec3 volExtentMin = vec3(-1.0);
	vec3 volExtentMax = vec3(1.0);
	// vec4 clipPos = ndcPos / gl_FragCoord.w;
	// vec4 eyePos = inv_proj * clipPos;
	// vec4 ray_view = vec4(eyePos.xy, -1.0, 0.0);
	// vec3 worldDir = (inv_view*ray_view).xyz;
	// worldDir = normalize(worldDir);

	vec4 dst = vec4(vec3(1.0), 0.0);
	vec3 cam_position = vec3(inv_vp[3]);
	vec3 direction = normalize(gl_FragCoord.xyz-cam_position);
	// vec3 position = ray_view.xyz;
	// vec3 direction = worldDir;
	vec3 position = gl_FragCoord.xyz;
	// vec3 direction = gl_FragCoord.xyz-cam_pos;
	// direction = normalize(direction);
	vec4 value = vec4(0.0);
	float scalar = 0;
	float step_size = 0.01;
	for (int i = 0; i < 200; i++)
	{
		// value = texture(perlin, position);
		// scalar = (value.r + value.g + value.b)/3.0;
		// scalar = value.a
		// dst.a += 0.01;
		// dst.a += (1.0-dst.a) * scalar*0.5;
		// dst.a += scalar*0.01;
		dst.a += 0.05;
		position = position+direction*step_size;
		vec3 temp1 = sign(position-volExtentMin);
		vec3 temp2 = sign(volExtentMax-position);
		float inside = dot(temp1, temp2);
		if (inside < 3.0) {
			break;
		}

	}
	// fragColor = vec4(direction, 1.0);

	fragColor = dst;
}