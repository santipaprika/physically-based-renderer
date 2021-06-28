
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform samplerCube u_texture;
uniform vec3 u_camera_position;

void main()
{
	vec3 dir = normalize(v_world_position - u_camera_position);
	vec3 reflected_dir = -reflect(dir,v_normal);
	vec4 color = textureCube(u_texture, reflected_dir);

	gl_FragColor = color;
}
