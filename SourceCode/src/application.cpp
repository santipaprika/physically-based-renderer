#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "includes.h"

#include <cmath>

Application* Application::instance = NULL;
Camera* Application::camera = nullptr;

Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;
	render_wireframe = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(15.f, 15.0f, 25.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f);

	//// Create lights
	Light* light_0 = new Light();
	light_0->position = Vector3(-15, 5, -15);
	light_0->name = "Light 1";
	lights.push_back(light_0);
	
	Light* light_1 = new Light();
	light_1->position = Vector3(15, 15, 0);
	light_1->name = "Light 2";
	lights.push_back(light_1);

	// Create scene node 
	SceneNode * node = new SceneNode("Rendered node");
	root.push_back(node);

	// Set mesh and manipulate model matrix
	node->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");
	node->model.setScale(2, 2, 2);

	HDRE* hdre = new HDRE("data/environments/environment.hdre");
	//Texture* cubemapTex = new Texture();
	//cubemapTex->createCubemap(hdre->width, hdre->height, (uint8**)hdre->getFaces(0));

	// Create node material
	PBRMaterial * material = new PBRMaterial(hdre);
	node->material = material;
	material->environment = hdre;
	// Manipulate material
	material->color = vec4(1.0, 1.0, 1.0, 1.0);
	
	material->use_properties[PUNCTUAL_LIGHT] = true;
	material->use_properties[IBL] = true;

	material->brdfLUT = Texture::Get("data/brdfLUT.png");
	//material->texture = cubemapTex;
	
	material->albedo_map = Texture::Get("data/maps/albedo_map.png");
	material->use_properties[ALBEDO_MAP] = true;

	material->normal_map = Texture::Get("data/maps/normal_map.png");
	material->use_properties[NORMAL_MAP] = true;

	material->rough_map = Texture::Get("data/maps/roughness_map.png");
	material->use_properties[ROUGH_MAP] = true;

	material->metal_map = Texture::Get("data/maps/metal_map.png");
	material->use_properties[METAL_MAP] = true;

	material->opacity_map = Texture::Get("data/maps/opacity_map.png");
	material->use_properties[OPACITY_MAP] = true;

	material->occlusion_map = Texture::Get("data/maps/occlusion_map.png");
	material->use_properties[OCCLUSION_MAP] = true;

	material->emission_map = Texture::Get("data/maps/emission_map.png");
	material->use_properties[EMISSION_MAP] = true;

	material->heigh_map = Texture::Get("data/maps/heigh_map.png");
	material->use_properties[HEIGH_MAP] = false;	//not working very well

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Application::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	for (int i = 0; i < root.size(); i++) {
		root[i]->render(camera);

		if(render_wireframe)
			root[i]->renderWireframe(camera);
	}

	//Draw the floor grid
	if(render_debug)
		drawGrid();
}

void Application::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * 10; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 0.5;
	
	//example
	/*float angle = (float)seconds_elapsed * 10.0f*DEG2RAD;
	for (int i = 0; i < root.size(); i++) {
		root[i]->model.rotate(angle, Vector3(0,1,0));
	}*/

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT && !ImGui::IsAnyWindowHovered() 
		&& !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())) //is left button pressed?
	{
		camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
	}

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f,  1.0f, 0.0f) * speed);

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

//Keyboard event handler (sync input)
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: render_debug = !render_debug; break;
		case SDLK_F5: Shader::ReloadAll(); break; 
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event)
{
}

void Application::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Application::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
		case SDL_MOUSEWHEEL:
		{
			if (event.x > 0) io.MouseWheelH += 1;
			if (event.x < 0) io.MouseWheelH -= 1;
			if (event.y > 0) io.MouseWheel += 1;
			if (event.y < 0) io.MouseWheel -= 1;
		}
	}

	if(!ImGui::IsAnyWindowHovered() && event.y)
		camera->changeDistance(event.y * 0.5);
}

void Application::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

