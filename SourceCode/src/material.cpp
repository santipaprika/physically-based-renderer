#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);

	shader->setUniform("u_color", color);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera * camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

ReflectiveMaterial::ReflectiveMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/reflective.fs");
}

ReflectiveMaterial::~ReflectiveMaterial()
{

}

void ReflectiveMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

PhongMaterial::PhongMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
}

PhongMaterial::~PhongMaterial()
{

}

void PhongMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);

	shader->setUniform("u_color", color);

	//Material properties
	shader->setUniform("u_ka", ka);
	shader->setUniform("u_kd", kd);
	shader->setUniform("u_ks", ks);

	//Scene light properties
	shader->setUniform("u_ia", ia);
	shader->setUniform("u_id", id);
	shader->setUniform("u_is", is);

	//material shininess
	shader->setUniform("u_shininess", shininess);

	//send lights to shader
	shader->setUniform("u_lights_position_1", Application::instance->lights[0]->position);
	shader->setUniform("u_lights_position_2", Application::instance->lights[1]->position);

	if (texture)
		shader->setUniform("u_texture", texture);
}

void PhongMaterial::renderInMenu()
{
	ImGui::SliderFloat("Shininess", (float*)&shininess, 1, 50);
	ImGui::ColorEdit3("Ka", (float*)&ka); // Edit 3 floats representing a color
	ImGui::ColorEdit3("Kd", (float*)&kd);
	ImGui::ColorEdit3("Ks", (float*)&ks);

	ImGui::ColorEdit3("Ia", (float*)&ia);
	ImGui::ColorEdit3("Id", (float*)&id);
	ImGui::ColorEdit3("Is", (float*)&is);

}

PBRMaterial::PBRMaterial(HDRE* environment)
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skeleton_pbr.fs");

	roughness = 0.29;
	emission_factor = 0.1;
	metallic_factor = 0.1;
	occlusion_factor = 1;

	albedo_map = NULL;
	rough_map = NULL;
	metal_map = NULL;
	normal_map = NULL;
	emission_map = NULL;
	occlusion_map = NULL;

	sHDRELevel level = environment->getLevel(0);
	Texture* cubemapTex = new Texture();
	cubemapTex->createCubemap(level.width, level.height, (uint8**)level.faces);
	prem_levels.push_back(cubemapTex);

	sHDRELevel level1 = environment->getLevel(1);
	Texture* cubemapTex1 = new Texture();
	cubemapTex1->createCubemap(level1.width, level1.height, (uint8**)level1.faces);
	prem_levels.push_back(cubemapTex1);

	sHDRELevel level2 = environment->getLevel(2);
	Texture* cubemapTex2 = new Texture();
	cubemapTex2->createCubemap(level2.width, level2.height, (uint8**)level2.faces);
	prem_levels.push_back(cubemapTex2);

	sHDRELevel level3 = environment->getLevel(3);
	Texture* cubemapTex3 = new Texture();
	cubemapTex3->createCubemap(level3.width, level3.height, (uint8**)level3.faces);
	prem_levels.push_back(cubemapTex3);

	sHDRELevel level4 = environment->getLevel(4);
	Texture* cubemapTex4 = new Texture();
	cubemapTex4->createCubemap(level4.width, level4.height, (uint8**)level4.faces);
	prem_levels.push_back(cubemapTex4);

	sHDRELevel level5 = environment->getLevel(5);
	Texture* cubemapTex5 = new Texture();
	cubemapTex5->createCubemap(level5.width, level5.height, (uint8**)level5.faces);
	prem_levels.push_back(cubemapTex5);
}

PBRMaterial::~PBRMaterial()
{
	
}

void PBRMaterial::setUniforms(Camera* camera, Matrix44 model)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);

	//upload node uniforms
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);

	//color
	shader->setUniform("u_color", color);

	//HDRE levels
	shader->setUniform("u_texture", prem_levels[0],0);
	shader->setUniform("u_texture_prem_0", prem_levels[1],1);
	shader->setUniform("u_texture_prem_1", prem_levels[2],2);
	shader->setUniform("u_texture_prem_2", prem_levels[3],3);
	shader->setUniform("u_texture_prem_3", prem_levels[4],4);
	shader->setUniform("u_texture_prem_4", prem_levels[5],5);

	//BRDF LUT
	shader->setUniform("u_brdf_lut", brdfLUT, 6);

	//albedo map
	shader->setUniform("u_albedo_map", albedo_map, 7);

	//roughness & roughness map	
	shader->setUniform("u_roughness", roughness);
	shader->setUniform("u_roughness_map", rough_map, 8);

	//metallic & metallic map
	shader->setUniform("u_metallic_fact", metallic_factor);
	shader->setUniform("u_metal_map",metal_map, 9);

	//normal map
	shader->setUniform("u_normal_map",normal_map, 10);

	//opacity map
	shader->setUniform("u_opacity_map", opacity_map, 11);

	//emission & emission map
	shader->setUniform("u_emission_fact", emission_factor);
	shader->setUniform("u_emission_map", emission_map, 12);

	//occlusion map
	shader->setUniform("u_occlusion_factor", occlusion_factor);
	shader->setUniform("u_occlusion_map", occlusion_map, 13);

	//heigh map
	shader->setUniform("u_heigh_map", heigh_map, 14);

	//send lights to shader
	shader->setUniform("u_lights_position_1", Application::instance->lights[0]->position);

	//bool vector
	shader->setUniform("u_use_punctual_light", use_properties[PUNCTUAL_LIGHT]);
	shader->setUniform("u_use_ibl", use_properties[IBL]);
	shader->setUniform("u_use_rough_map", use_properties[ROUGH_MAP]);
	shader->setUniform("u_use_albedo", use_properties[ALBEDO_MAP]);
	shader->setUniform("u_use_rough_map", use_properties[ROUGH_MAP]);
	shader->setUniform("u_use_metal_map", use_properties[METAL_MAP]);
	shader->setUniform("u_use_normal_map", use_properties[NORMAL_MAP]);
	shader->setUniform("u_use_opacity_map", use_properties[OPACITY_MAP]);
	shader->setUniform("u_use_emission_map", use_properties[EMISSION_MAP]);
	shader->setUniform("u_use_occlusion_map", use_properties[OCCLUSION_MAP]);
	shader->setUniform("u_use_heigh_map", use_properties[HEIGH_MAP]);

	//if (texture)
	//	shader->setUniform("u_texture", texture);
}

void PBRMaterial::renderInMenu()
{
	ImGui::Checkbox("Punctual Light", &use_properties[PUNCTUAL_LIGHT]);
	ImGui::Checkbox("IBL", &use_properties[IBL]);

	ImGui::SliderFloat("Roughness", (float*)&roughness, 0.01, 0.99);
	ImGui::SliderFloat("Metallic Factor", (float*)&metallic_factor, 0.01, 0.99);
	ImGui::SliderFloat("Occlusion Factor", (float*)&occlusion_factor, 0.01, 0.99);
	ImGui::SliderFloat("Emission Factor", (float*)&emission_factor, 0.01, 0.99);
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
	
	ImGui::Checkbox("Albedo Map", &use_properties[ALBEDO_MAP]);
	ImGui::Checkbox("Normal Map", &use_properties[NORMAL_MAP]);
	ImGui::Checkbox("Roughness Map", &use_properties[ROUGH_MAP]);
	ImGui::Checkbox("Metalness Map", &use_properties[METAL_MAP]);
	ImGui::Checkbox("Opacity Map", &use_properties[OPACITY_MAP]);
	ImGui::Checkbox("Occlusion Map", &use_properties[OCCLUSION_MAP]);
	ImGui::Checkbox("Emission Map", &use_properties[EMISSION_MAP]);
	ImGui::Checkbox("Heigh Map", &use_properties[HEIGH_MAP]);
}