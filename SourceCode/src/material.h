#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	vec4 color;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

class ReflectiveMaterial : public StandardMaterial {
public:

	ReflectiveMaterial();
	~ReflectiveMaterial();
	void renderInMenu();
};

class PhongMaterial : public StandardMaterial {
public:

	//Material properties
	Vector3 ka = Vector3(1,0,0);
	Vector3 kd = Vector3(0.5,0,0);
	Vector3 ks = Vector3(1,1,1);

	//Scene light properties
	Vector3 ia = Vector3(0.05, 0.05, 0.05);
	Vector3 id = Vector3(0.5, 0.5, 0.5);
	Vector3 is = Vector3(0.5, 0.5, 0.5);

	//Material shininess
	float shininess = 6;

	PhongMaterial();
	~PhongMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void renderInMenu();
};

enum{PUNCTUAL_LIGHT, IBL, ALBEDO_MAP, ROUGH_MAP, METAL_MAP, NORMAL_MAP, OPACITY_MAP, EMISSION_MAP, OCCLUSION_MAP, HEIGH_MAP};
class PBRMaterial : public StandardMaterial {
public:
	
	bool use_properties[9] = { 0,0,0,0,0,0,0,0,0 };
	//Material properties
	HDRE* environment;
	//Texture* cubemapTex;
	std::vector<Texture*> prem_levels;

	//BRDF LUT
	Texture* brdfLUT;

	//in case of color mapping
	Texture* albedo_map; //this is an image indicating the color for each coordinate

	float roughness;	//for constant values
	Texture* rough_map; //this is an image indicating the roughness for each coordinate

	float metallic_factor;	//for constant values
	Texture* metal_map; //this is an image indicating the metallicity? for each coordinate

	Texture* normal_map; //this is an image indicating the normal for each coordinate

	Texture* opacity_map;

	float emission_factor;	//for constant values
	Texture* emission_map;  //this is an image indicating the emissive map for each coordinate (that will be updated as the scene is computed)

	float occlusion_factor;
	Texture* occlusion_map; //this is an image indicating the occlusion map ((that will be updated as the scene is computed)

	Texture* heigh_map;

	PBRMaterial(HDRE* environment);
	~PBRMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void renderInMenu();
};


#endif