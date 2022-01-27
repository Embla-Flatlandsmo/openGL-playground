#version 430 core

uniform layout(location = 1) mat4 VP;
layout (location = 0) in vec3 vert;
layout (location = 1) in vec4 pos;
layout (location = 2) in vec4 vel;
layout (location = 3) in vec4 col;

out vec4 vert_color;

/**
 * Create rotation matrix from field vector.
 * The returned matrix can rotate vector (1, 0, 0)
 * into the desired setup. Used to rotate glyphs according
 * to vecotr field
 * http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
 */
mat4 getRotationMat(vec3 vector)
{
	vec3 unit = vec3(1, 0, 0);
	vec3 f = normalize(vector);
	vec3 cross = cross(f, unit);
	vec3 a = normalize(cross);
	float s = length(cross);
	float c = dot(f, unit);
	float oc = 1.0 - c;

	return mat4(oc * a.x * a.x + c,        oc * a.x * a.y - a.z * s,  oc * a.z * a.x + a.y * s,  0.0,
                oc * a.x * a.y + a.z * s,  oc * a.y * a.y + c,        oc * a.y * a.z - a.x * s,  0.0,
                oc * a.z * a.x - a.y * s,  oc * a.y * a.z + a.x * s,  oc * a.z * a.z + c,        0.0,
                0.0,                       0.0,                       0.0,                       1.0);

}

void main() {
	mat4 rot = getRotationMat(vel.xyz);
	vec3 rvert = vec3(rot * vec4(vert.xyz, 1.0f));
 	gl_Position = VP * vec4(rvert + pos.xyz, 1.0);
	// gl_Position = VP*vec4(position.xyz, 1.0);
	vert_color = col;
	// colorVal = (clamp(colorValue, minColorValue, maxColorValue) - minColorValue) / max(1, (maxColorValue - minColorValue));
}