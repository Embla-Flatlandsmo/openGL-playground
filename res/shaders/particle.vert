#version 430 core

uniform layout(location = 1) mat4 VP;
uniform layout(location = 2) float particleSize;

in layout (location = 0) vec3 vert;
in layout (location = 1) vec3 normal_in;
in layout (location = 2) vec2 textureCoordinates_in;
in layout (location = 3) vec4 pos;
in layout (location = 4) vec4 vel;
in layout (location = 5) vec4 acc;

out vec4 vert_color;

/**
 * Create rotation matrix from field vector.
 * The returned matrix can rotate vector (0, 1, 0)
 * into the desired setup. Used to rotate the object according 
 * to its heading
 * http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
 */
mat4 getRotationMat(vec3 vector)
{
	vec3 unit = vec3(0, 1, 0);
	vec3 f = normalize(vector);
	vec3 x = cross(f, unit);
	vec3 a = normalize(x);
	float s = length(x);
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
 	gl_Position = VP * vec4(particleSize*rvert + pos.xyz, 1.0);
	vert_color = vec4(clamp(normalize(abs(vel.xyz)), vec3(0.2), vec3(1.0)), 1.0);
}