#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
}


SceneNode::SceneNode(const char * name)
{
	this->name = name;
}

SceneNode::~SceneNode()
{

}

void SceneNode::render(Camera* camera)
{
	if (material)
		material->render(mesh, model, camera);
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
		
		ImGui::TreePop();
	}

	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		material->renderInMenu();
		ImGui::TreePop();
	}
}

Skybox::Skybox()
{
	this->name = std::string("Skybox");

	mesh = new Mesh();
	mesh->createCube();
	
	material = new StandardMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/cubemap.fs");

	model.scale(100, 100, 100);
}

Skybox::Skybox(Texture * tex)
{
	this->name = std::string("Skybox");

	mesh = new Mesh();
	mesh->createCube();

	material = new StandardMaterial();
	material->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/cubemap.fs");
	material->texture = tex;

	model.scale(100, 100, 100);
}

Skybox::~Skybox()
{

}

void Skybox::render(Camera* camera)
{
	model.setTranslation(camera->eye[0], camera->eye[1], camera->eye[2]);

	if (mesh && material)
	{
		glDisable(GL_DEPTH_TEST);
		material->render(mesh, model, camera);
		glEnable(GL_DEPTH_TEST);
	}
}

Light::Light()
{
	this->name = "Light";
	position = vec3(10, 10, 10);
	color = vec3(1.f, 1.f, 1.f);
	intensity = 1.f;
}

void Light::renderInMenu()
{
	{
		ImGui::DragFloat3("Position", (float*)&position, 0.1f);
		ImGui::ColorEdit3("Color", (float*)&color, 0.1f);
		ImGui::DragFloat("Scale", &intensity, 0.1f, 0, 10);
	}
}