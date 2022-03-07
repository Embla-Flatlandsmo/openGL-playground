// #version 430 core
// uniform vec4 viewport;
// uniform mat4 inverse_vp;
// uniform vec3 eyePos;
// uniform vec3 boxHigh;
// uniform vec3 boxLow;

// out vec4 fragColor;

// struct ray_t
// {
//     vec3 origin;
//     vec3 direction;
// };

// bool hit_sphere(vec3 center, float radius, vec3 ro, vec3 rd) {
//     vec3 oc = ro - center;
//     float a = dot(rd, rd);
//     float b = 2.0 * dot(oc, rd);
//     float c = dot(oc, oc) - radius*radius;
//     float discriminant = b*b - 4*a*c;
//     return (discriminant > 0);
// }

// void main()
// {
//     // vec4 ndcPos;
//     // ndcPos.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
//     // ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
//     //     (gl_DepthRange.far - gl_DepthRange.near);
//     // ndcPos.w = 1.0;

//     // vec4 clipPos = ndcPos / gl_FragCoord.w;
//     // vec4 eyePos = inverse_vp * clipPos;
//     vec4 screen_space_near = vec4(gl_FragCoord.xy, 0.0, 0.0);
//     vec4 screen_space_far = vec4(gl_FragCoord.xy, 1.0, 0.0);

//     vec4 far = inverse_vp*screen_space_far;
//     far /= far.w;
//     vec4 near = inverse_vp*screen_space_near;
//     near /= near.w;
//     vec3 rayOrigin = eyePos;
//     vec3 rayDirection = normalize(far.xyz-near.xyz);
//     if (hit_sphere(vec3(0.), 300, rayOrigin, rayDirection)) {
//         fragColor = vec4(1.0);
//     } else {
//         fragColor = vec4(vec3(0.0),1.0);
//     }
// }


// void main() {
//     vec4 ndcPos;
//     ndcPos.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
//     ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
//         (gl_DepthRange.far - gl_DepthRange.near);
//     ndcPos.w = 1.0;

//     vec4 clipPos = ndcPos / gl_FragCoord.w;
//     vec4 eyePos = invPersMatrix * clipPos;

//     if ((eyePos.x < boxHigh.x) && (eyePos.y < boxHigh.y) && (eyePos.z < boxHigh.z)) {
//         fragColor = vec4(vec3(0.0),1.0);
//     } else {
//         fragColor = vec4(1.0);
//     }
// }

#version 430 core



// in vec2 fragCoord;
in vec4 gl_FragCoord;
out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;
uniform mat4 inverse_mvp;
uniform vec3 bowLow;
uniform vec3 boxHigh;
//-----------------------------------------------------------------------------
// Maths utils
//-----------------------------------------------------------------------------
mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p );
    return f;
}


//-----------------------------------------------------------------------------
// Main functions
//-----------------------------------------------------------------------------
float scene(vec3 p)
{	
	return .1-length(p)*.05+fbm(p*.3);
}

void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 v = -1.0 + 2.0*q;
    v.x *= iResolution.x/ iResolution.y;

	#if 0
    vec2 mo = -1.0 + 2.0*iMouse.xy / iResolution.xy;
    #else
	vec2 mo = vec2(iTime*.1,cos(iTime*.25)*3.);
	#endif

    // camera by iq
    vec3 org = 25.0*normalize(vec3(cos(2.75-3.0*mo.x), 0.7-1.0*(mo.y-1.0), sin(2.75-3.0*mo.x)));
	vec3 ta = vec3(0.0, 1.0, 0.0);
    vec3 ww = normalize( ta - org);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 dir = normalize( v.x*uu + v.y*vv + 1.5*ww );
	vec4 color=vec4(.0);
	
	
	
	const int nbSample = 64;
	const int nbSampleLight = 6;
	
	float zMax         = 40.;
	float stp         = zMax/float(nbSample);
	float zMaxl         = 20.;
	float stepl         = zMaxl/float(nbSampleLight);
    vec3 p             = org;
    float T            = 1.;
    float absorption   = 100.;
	vec3 sun_direction = normalize( vec3(1.,.0,.0) );
    
	for(int i=0; i<nbSample; i++)
	{
		float density = scene(p);
		if(density>0.)
		{
			float tmp = density / float(nbSample);
			T *= 1. -tmp * absorption;
			if( T <= 0.01)
				break;
				
				
			 //Light scattering
			float Tl = 1.0;
			for(int j=0; j<nbSampleLight; j++)
			{
				float densityLight = scene( p + normalize(sun_direction)*float(j)*stepl);
				if(densityLight>0.)
                	Tl *= 1. - densityLight * absorption/float(nbSample);
                if (Tl <= 0.01)
                    break;
			}
			
			//Add ambiant + light scattering color
			color += vec4(1.)*50.*tmp*T +  vec4(1.,.7,.4,1.)*80.*tmp*T*Tl;
		}
		p += dir*stp;
	}    

    fragColor = color;
    // float brightness = length(smoothstep(vec2(0.0), iResolution, gl_FragCoord.xy));
    // fragColor = vec4(brightness, brightness, brightness, 1.0);

}