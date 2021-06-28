#ifndef SCENENODE_H
#define SCENENODE_H

#include "framework.h"

#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "material.h"


class Light;

class SceneNode {
public:

	static unsigned int lastNameId;

	SceneNode();
	SceneNode(const char* name);
	~SceneNode();

	Material * material = NULL;
	std::string name;

	Mesh* mesh = NULL;
	Matrix44 model;

	Light* node_light;

	virtual void render(Camera* camera);
	virtual void renderWireframe(Camera* camera);
	virtual void renderInMenu();
};

class Skybox : public SceneNode {
public:
	
	Skybox();
	Skybox(Texture * texture);
	~Skybox();

	void render(Camera* camera);
	void renderInMenu() {}
};

class Light : public SceneNode {
public:

	Light();
	~Light();

	vec3 position;
	vec3 color;
	float intensity;

	void renderInMenu();
};

#endif