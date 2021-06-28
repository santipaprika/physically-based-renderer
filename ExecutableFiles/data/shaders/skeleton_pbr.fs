
// declare here your constants
#define PI				3.14159265359
#define RECIPROCAL_PI	0.3183098861837697 // 1 / PI

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;

uniform vec3 u_camera_position;
uniform vec4 u_color;

// Levels of HDRE
uniform samplerCube u_texture;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

// BRDF LUT
uniform sampler2D u_brdf_lut;

//variables use boolean
uniform bool u_use_punctual_light;
uniform bool u_use_ibl;

uniform bool u_use_albedo;
uniform bool u_use_rough_map;
uniform bool u_use_metal_map;
uniform bool u_use_normal_map;
uniform bool u_use_opacity_map;
uniform bool u_use_emission_map;
uniform bool u_use_occlusion_map;
uniform bool u_use_heigh_map;

// Albedo map
uniform sampler2D u_albedo_map;

// roughness
uniform float u_roughness;
uniform sampler2D u_roughness_map;

// Metallic
uniform float u_metallic_fact;
uniform sampler2D u_metal_map;

// Normal Map
uniform sampler2D u_normal_map;

// Opacity Map
uniform sampler2D u_opacity_map;

// Emission
uniform float u_emission_fact;
uniform sampler2D u_emission_map;

// Occlusion
uniform float u_occlusion_factor;
uniform sampler2D u_occlusion_map;

// Heigh
uniform sampler2D u_heigh_map;

// Lights
uniform vec3 u_lights_position_1;


// Material struct
struct Material
{
  vec4 color;
  float roughness;
  float metalness;
  float occlusion;
  float emission;
  
  vec3 f_diffuse;
  vec3 f_specular;
} thisMaterial;

// don't touch this
// getReflectionColor:	Get pixel from HDRE (passed in separated textures)
//						using a 3D direction and a defined roughness
vec3 getReflectionColor(vec3 r, float roughness)
{
	float lod = roughness * 5.0;

	vec4 color;

	if(lod < 1.0) color = mix( textureCube(u_texture, r), textureCube(u_texture_prem_0, r), lod );
	else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, r), textureCube(u_texture_prem_1, r), lod - 1.0 );
	else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, r), textureCube(u_texture_prem_2, r), lod - 2.0 );
	else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, r), textureCube(u_texture_prem_3, r), lod - 3.0 );
	else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, r), textureCube(u_texture_prem_4, r), lod - 4.0 );
	else color = textureCube(u_texture_prem_4, r);

	color = pow(color, vec4(1.0/2.2));

	return color.rgb;
}

// don't touch these neither
// perturbNormal:	Modify material normal using normal texture
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv){
	
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );

	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}
vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel ){

	normal_pixel = normal_pixel * 255./127. - 128./127.;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}


vec3 computePunctualLight(vec3 N, vec3 V, vec3 L, vec3 H){
	
	// (punctual diffuse component)
	vec3 f_lambert = thisMaterial.f_diffuse / PI;	//diffuse
	
	vec3 F = thisMaterial.f_specular + (1 - thisMaterial.f_specular)*pow(1-dot(L,N),5);
	
	// distribution function
	float alpha = pow(thisMaterial.roughness,2);
	float D = pow(alpha,2) / (PI * pow((pow(dot(N,H),2) * (pow(alpha,2) - 1) + 1),2));
	
	// geometry distribution function	
	float k = pow(thisMaterial.roughness + 1, 2) / 8;
	float G_1_v = dot(N,V) / (dot(N,V) * (1 - k) + k) ;
	float G_1_l = dot(N,L) / (dot(N,L) * (1 - k) + k) ;
	float G = G_1_l * G_1_v;
	
	//BRDF facet (punctual specular component)
	vec3 f_pfacet = (F * G * D) / (4 * dot(N,L) * dot(N,V));
	
	//Punctual light BRDF
	vec3 f_pl = f_lambert + f_pfacet;
	
	return f_pl * clamp(dot(L,N),0.000001,0.999999);
}

vec3 computeIBL(vec3 N, vec3 V, vec3 L){
	
	//store albedo information
	vec2 xy =  vec2(thisMaterial.roughness,clamp(dot(N,L),0.01,0.99));
	vec4 LUT_value = texture2D(u_brdf_lut,xy);
	
	//IBL specular component
	float A = LUT_value.x;
	
	//IBL diffuse component
	float B = LUT_value.y;
	
	//IBL BRDF 
	vec3 f_ibl = (thisMaterial.f_specular * A + B);
	
	vec3 prem_color = thisMaterial.occlusion * getReflectionColor((reflect(V,N)),thisMaterial.roughness);

	return f_ibl * prem_color;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture2D(u_heigh_map, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * 1);
    return texCoords - p;    
} 

void setMaterialProperties(vec2 uv){
	
	// Initialize
	thisMaterial.color;
	thisMaterial.roughness = 0.1;
	thisMaterial.metalness = 0.1;
	thisMaterial.occlusion = 0.1;
	thisMaterial.emission = 0.1;
	
	// Roughness
	if (u_use_rough_map)	thisMaterial.roughness = texture2D(u_roughness_map, uv).x;
	else	thisMaterial.roughness = u_roughness;
	
	// Metalness
	if (u_use_metal_map)	thisMaterial.metalness = texture2D(u_metal_map, uv ).b;
	else	thisMaterial.metalness = u_metallic_fact;
	
	// Albedo
	if (u_use_albedo)	thisMaterial.color = texture2D(u_albedo_map, uv);
	else	thisMaterial.color = u_color;
	
	// Opacity
	if (u_use_opacity_map)	thisMaterial.color.a = texture2D(u_opacity_map, uv).x;
	
	// Occlusion
	if (u_use_occlusion_map)	thisMaterial.occlusion = texture2D(u_occlusion_map, uv).x;
	else	thisMaterial.occlusion = u_occlusion_factor;
	
	// Emission
	if (u_use_emission_map)	thisMaterial.emission = texture2D(u_emission_map, uv).x;
	else	thisMaterial.emission = u_emission_fact;
	
	// personalized way to apply emission (probably not the correct one, but it does something similar)
	thisMaterial.occlusion = thisMaterial.occlusion + thisMaterial.emission;
	
	//Diffuse and specular components of the material
	thisMaterial.f_diffuse = mix(vec3(0), thisMaterial.color, 1 - thisMaterial.metalness).rgb;
	thisMaterial.f_specular = mix(vec3(0.04), thisMaterial.color, thisMaterial.metalness).rgb;
}

void main()
{
	vec3 world_position = v_world_position;
	
	//light position
	vec3 light_pos = u_lights_position_1;
	
	//computeVectors
	vec3 V = normalize(u_camera_position - world_position);
	vec3 N = normalize(v_normal);
	vec3 L = normalize(light_pos - world_position);
	vec3 H = normalize(V + L);
	
	vec2 uv = v_uv * 3;
	
	//1st try of height map
	//vec2 uv = ParallaxMapping(v_uv,V) * 3;

	//use maps to define material properties and the f's.
	setMaterialProperties(uv);
	
	//2nd try of height map (we try to move the world position towards the normal direction)
	if (u_use_heigh_map){
		float step = texture2D(u_heigh_map, uv).r;
		vec3 step3 = normalize(v_normal) * step;
		world_position = v_world_position + step3;
	}

	//normal map
	if (u_use_normal_map){
		vec3 normal_pixel = texture2D(u_normal_map, uv).rgb;
		N = perturbNormal(normalize(v_normal),V,uv,normal_pixel);
	}
	
	
	// ----------------------------- PUNCTUAL LIGHT -------------------------------
	
	vec3 color_pl = vec3(0,0,0);
	if (u_use_punctual_light) color_pl = computePunctualLight(N,V,L,H);
	
	// -------------------------------- IBL ----------------------------------------
	
	vec3 color_ibl = vec3(0,0,0);
	if (u_use_ibl) color_ibl = computeIBL(N,V,L);
	
	
	//Sum punctual light and IBL
	thisMaterial.color.rgb = color_pl + color_ibl;
	
	// simple tone-mapping
	//thisMaterial.color.rgb /= (thisMaterial.color.rgb + vec3(1.0));
	
	// gamma correct 
	//color.rgb = pow(color.rgb, vec3(1.0/2.2));

	// Final color
	gl_FragColor = thisMaterial.color;
}