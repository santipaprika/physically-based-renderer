
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform samplerCube u_texture;
uniform vec3 u_camera_position;

//Material properties
uniform vec3 u_ka;
uniform vec3 u_kd;
uniform vec3 u_ks;

//Scene light properties
uniform vec3 u_ia;
uniform vec3 u_id;
uniform vec3 u_is;

//shininess
uniform float u_shininess;

//Lights
uniform int u_num_lights;
uniform vec3 u_lights_position_1;
uniform vec3 u_lights_position_2;


void main()
{
	//create array of lights
	vec3 lights_positions[2];
	lights_positions[0] = u_lights_position_1;
	lights_positions[1] = u_lights_position_2;
	
	//normalize normal received from vertex shader
	vec3 normal = normalize(v_normal);
	
	//eye vector
	vec3 V = normalize(u_camera_position - v_world_position);
	
	int num_lights = lights_positions.length();
	
	//ambient component
	vec3 ambient_component = u_ka * u_ia;
	vec3 diffuse_specular_term = vec3(0);
	
	for (int i = 0; i < num_lights; i++)
	{
		//light vector
		vec3 L = normalize(lights_positions[i] - v_world_position);
		
		//reflected vector
		vec3 R = -reflect(L,normal);
		
		//diffuse component
		vec3 diffuse_component = u_kd * clamp(dot(L,normal),0,1) * u_id;

		//specular component
		vec3 specular_component = u_ks * pow(clamp(dot(R,V),0,1),u_shininess) * u_is;

		//sum for each light
		diffuse_specular_term += diffuse_component + specular_component;
	}
	
	vec3 Ip = ambient_component + diffuse_specular_term;

	gl_FragColor = vec4(Ip, 1);
}
